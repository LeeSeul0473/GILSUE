# 서버 IP 자동 등록/조회 기능 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 언리얼 리슨 서버가 Lobby 진입 시 자신의 IP/Port를 웹서버에 등록하고, 클라이언트는 로그인 성공 직후 그 정보를 받아와 수동 IP 입력 없이 접속하게 한다.

**Architecture:** FastAPI 웹서버에 단일 슬롯(`idx=1` 고정 행)짜리 `game_server` 테이블과 `/server/register`, `/server/info` 엔드포인트를 추가한다. 언리얼 쪽 `UWebApiSubsystem`에 두 HTTP 호출 함수를 추가하고, `LobbyGM::BeginPlay`(서버 전용)에서 등록을 호출하며, `TitleWidgetBase`가 로그인 성공 직후 조회해 `ConnectServer()`의 접속 대상으로 사용한다.

**Tech Stack:** Unreal Engine 5 C++ (HttpModule, Json), Python 3 / FastAPI / pymysql / MySQL, pytest + httpx(TestClient)

**Spec:** `Task.md` (프로젝트 루트) — 이 계획이 구현하는 설계 결정 전체가 여기 있음

## Global Constraints

- 언리얼 엔진 관련 작업은 C++로만 한다 (Blueprint 금지) — `docs/required.md`.
- 언리얼-웹서버 통신은 HTTP + JSON만 사용한다. 소켓 통신 추가하지 않는다 — `docs/작업사항.md`.
- 등록 서버는 단일 슬롯만 유지한다. 다중 서버 목록, unregister 기능, 언리얼 쪽 IP 자동 탐지는 범위 밖이다 — `Task.md`.
- 기존 `ServerIP` 입력창은 웹서버 주소 용도로만 유지한다. 게임 서버 접속 주소는 더 이상 사용자가 직접 입력하지 않는다 — `Task.md`.

---

## File Structure

- Create: `Server/schema.sql` — `game_server` 테이블 정의 (수동 실행용)
- Create: `Server/tests/conftest.py` — 테스트에서 `Server/` 루트를 import 경로에 추가
- Create: `Server/tests/test_server.py` — `/server/register`, `/server/info` pytest
- Modify: `Server/requirements.txt` — `pytest`, `httpx` 추가
- Modify: `Server/main.py` — 요청/응답 모델 + 엔드포인트 2개 추가
- Modify: `Source/L20260713_Day03/Web/WebApiSubsystem.h` — 델리게이트/함수 선언 추가
- Modify: `Source/L20260713_Day03/Web/WebApiSubsystem.cpp` — HTTP 호출 구현 추가
- Modify: `Source/L20260713_Day03/Lobby/LobbyGM.h` — 등록 결과 핸들러 선언 추가
- Modify: `Source/L20260713_Day03/Lobby/LobbyGM.cpp` — `BeginPlay`에서 서버 등록 호출
- Modify: `Source/L20260713_Day03/DataGameInstanceSubsystem.h` — `GameServerIP`/`GameServerPort` 필드 추가
- Modify: `Source/L20260713_Day03/Title/TitleWidgetBase.h` — 조회 핸들러 선언 추가
- Modify: `Source/L20260713_Day03/Title/TitleWidgetBase.cpp` — 로그인 후 자동 조회 + `ConnectServer()` 변경

---

## Task 1: DB 스키마 (`game_server` 테이블)

**Files:**
- Create: `Server/schema.sql`

**Interfaces:**
- Produces: 테이블 `game_server(idx INT PRIMARY KEY, ip VARCHAR(45), port INT, registered_at DATETIME)`, 항상 `idx=1` 행 하나만 사용. Task 2가 이 스키마에 의존한다.

- [ ] **Step 1: 스키마 파일 작성**

`Server/schema.sql` 생성:

```sql
CREATE TABLE IF NOT EXISTS game_server (
    idx INT PRIMARY KEY,
    ip VARCHAR(45) NOT NULL,
    port INT NOT NULL,
    registered_at DATETIME NOT NULL
);
```

- [ ] **Step 2: MySQL Workbench에서 수동 실행**

`seul` 스키마를 선택한 상태에서 위 SQL을 실행한다 (기존 `member` 테이블도 같은 방식으로 만들어졌고, 이 프로젝트엔 마이그레이션 도구가 없다).

- [ ] **Step 3: 검증**

Workbench 쿼리창에서:

```sql
SELECT * FROM seul.game_server;
```

빈 결과(0 rows, 에러 없음)가 나오면 테이블 생성 완료.

- [ ] **Step 4: Commit**

```bash
git add Server/schema.sql
git commit -m "feat: add game_server table schema"
```

---

## Task 2: 웹서버 `/server/register`, `/server/info` 엔드포인트

**Files:**
- Modify: `Server/requirements.txt`
- Modify: `Server/main.py`
- Create: `Server/tests/conftest.py`
- Create: `Server/tests/test_server.py`

**Interfaces:**
- Consumes: `Server/db.py`의 `get_connection()` (기존 함수, 시그니처 변경 없음).
- Produces:
  - `POST /server/register` — body `{"port": int}` → `{"result": bool, "message": str}`. 클라이언트 IP는 서버가 `request.client.host`로 직접 읽는다(요청 body에 IP 없음).
  - `GET /server/info` — body 없음 → `{"result": bool, "ip": str, "port": int, "message": str}`.
  - Task 3(`WebApiSubsystem`)이 이 두 엔드포인트를 호출한다.

- [ ] **Step 1: 테스트용 의존성 추가**

`Server/requirements.txt`에 두 줄 추가:

```
pytest==8.3.4
httpx==0.28.1
```

- [ ] **Step 2: 테스트 경로 설정 파일 작성**

`Server/tests/conftest.py` 생성:

```python
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
```

- [ ] **Step 3: 실패하는 테스트 작성**

`Server/tests/test_server.py` 생성:

```python
from unittest.mock import MagicMock

from fastapi.testclient import TestClient

import main


def make_fake_conn(fetchone_result=None):
    cursor = MagicMock()
    cursor.__enter__.return_value = cursor
    cursor.__exit__.return_value = False
    cursor.fetchone.return_value = fetchone_result

    conn = MagicMock()
    conn.cursor.return_value = cursor
    return conn, cursor


def test_register_server_stores_caller_ip(monkeypatch):
    conn, cursor = make_fake_conn()
    monkeypatch.setattr(main, "get_connection", lambda: conn)

    client = TestClient(main.app)
    response = client.post("/server/register", json={"port": 7777})

    assert response.status_code == 200
    assert response.json() == {"result": True, "message": ""}

    execute_args = cursor.execute.call_args[0]
    assert execute_args[1] == ("testclient", 7777, "testclient", 7777)
    conn.commit.assert_called_once()


def test_register_server_rejects_non_positive_port(monkeypatch):
    conn, _cursor = make_fake_conn()
    monkeypatch.setattr(main, "get_connection", lambda: conn)

    client = TestClient(main.app)
    response = client.post("/server/register", json={"port": 0})

    assert response.status_code == 422


def test_server_info_returns_registered_server(monkeypatch):
    conn, _cursor = make_fake_conn(fetchone_result={"ip": "127.0.0.1", "port": 7777})
    monkeypatch.setattr(main, "get_connection", lambda: conn)

    client = TestClient(main.app)
    response = client.get("/server/info")

    assert response.status_code == 200
    assert response.json() == {
        "result": True,
        "ip": "127.0.0.1",
        "port": 7777,
        "message": "",
    }


def test_server_info_returns_false_when_nothing_registered(monkeypatch):
    conn, _cursor = make_fake_conn(fetchone_result=None)
    monkeypatch.setattr(main, "get_connection", lambda: conn)

    client = TestClient(main.app)
    response = client.get("/server/info")

    assert response.status_code == 200
    assert response.json() == {
        "result": False,
        "ip": "",
        "port": 0,
        "message": "등록된 서버가 없습니다",
    }
```

- [ ] **Step 4: 테스트 실행해서 실패 확인**

```bash
cd Server
pip install -r requirements.txt
pytest tests/test_server.py -v
```

Expected: `ModuleNotFoundError` 또는 `ImportError` (아직 엔드포인트가 없으므로 `import main`은 되지만 404 등으로 실패), 최소 4개 테스트 모두 FAIL.

- [ ] **Step 5: `main.py`에 모델과 엔드포인트 구현**

`Server/main.py`에서 `from db import get_connection` 아래, 기존 `AuthResponse` 클래스 다음에 모델 3개 추가:

```python
class RegisterServerRequest(BaseModel):
    port: int = Field(gt=0)


class RegisterServerResponse(BaseModel):
    result: bool
    message: str = ""


class ServerInfoResponse(BaseModel):
    result: bool
    ip: str = ""
    port: int = 0
    message: str = ""
```

파일 상단 import에 `Request` 추가 (`from fastapi import FastAPI` → `from fastapi import FastAPI, Request`).

기존 `/login` 엔드포인트 뒤에 엔드포인트 2개 추가:

```python
@app.post("/server/register", response_model=RegisterServerResponse)
def register_server(req: RegisterServerRequest, request: Request):
    ip = request.client.host
    conn = get_connection()
    try:
        with conn.cursor() as cur:
            cur.execute(
                "INSERT INTO game_server (idx, ip, port, registered_at)"
                " VALUES (1, %s, %s, NOW())"
                " ON DUPLICATE KEY UPDATE ip = %s, port = %s, registered_at = NOW()",
                (ip, req.port, ip, req.port),
            )
        conn.commit()
    finally:
        conn.close()

    return RegisterServerResponse(result=True)


@app.get("/server/info", response_model=ServerInfoResponse)
def get_server_info():
    conn = get_connection()
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT ip, port FROM game_server WHERE idx = 1")
            row = cur.fetchone()
    finally:
        conn.close()

    if row is None:
        return ServerInfoResponse(result=False, message="등록된 서버가 없습니다")

    return ServerInfoResponse(result=True, ip=row["ip"], port=row["port"])
```

- [ ] **Step 6: 테스트 실행해서 통과 확인**

```bash
pytest tests/test_server.py -v
```

Expected: 4개 테스트 모두 PASS.

- [ ] **Step 7: 실제 서버로 수동 확인**

```bash
run.bat
```

다른 터미널에서:

```bash
curl -X POST http://127.0.0.1:8080/server/register -H "Content-Type: application/json" -d "{\"port\":7777}"
curl http://127.0.0.1:8080/server/info
```

Expected: 첫 호출 `{"result":true,"message":""}`, 두번째 호출 `{"result":true,"ip":"127.0.0.1","port":7777,"message":""}`.

- [ ] **Step 8: Commit**

```bash
git add Server/requirements.txt Server/main.py Server/tests/conftest.py Server/tests/test_server.py
git commit -m "feat: add server register/info endpoints"
```

---

## Task 3: `UWebApiSubsystem`에 서버 등록/조회 함수 추가

**Files:**
- Modify: `Source/L20260713_Day03/Web/WebApiSubsystem.h`
- Modify: `Source/L20260713_Day03/Web/WebApiSubsystem.cpp`

**Interfaces:**
- Consumes: Task 2의 `/server/register`, `/server/info` 엔드포인트.
- Produces:
  - `void RequestRegisterServer(const FString& InWebServerIP, int32 InGamePort);` → 결과를 `OnRegisterServerResult`(기존 `FWebApiResultSignature`: `bool bInSuccess, const FString& InMessage`)로 브로드캐스트.
  - `void RequestServerInfo(const FString& InWebServerIP);` → 결과를 `OnServerInfoResult`(신규 `FWebApiServerInfoSignature`: `bool bInSuccess, const FString& InServerIP, int32 InServerPort, const FString& InMessage`)로 브로드캐스트.
  - Task 4(`LobbyGM`)가 `RequestRegisterServer`를 호출한다. Task 5(`TitleWidgetBase`)가 `RequestServerInfo`와 두 델리게이트를 모두 사용한다.

이 서브시스템에는 자동화 테스트가 없다(엔진 종속 HTTP 콜백이라 기존 `RequestLogin`/`RequestSignUp`도 테스트 없음, 기존 패턴 그대로 따름). Step 4에서 Task 2의 실제 서버로 수동 검증한다.

- [ ] **Step 1: 헤더에 델리게이트/함수 선언 추가**

`Source/L20260713_Day03/Web/WebApiSubsystem.h`의 `FWebApiResultSignature` 선언(10번째 줄) 바로 아래에 추가:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWebApiServerInfoSignature,
	const bool, bInSuccess, const FString&, InServerIP, const int32, InServerPort, const FString&, InMessage);
```

`OnSignUpResult` 프로퍼티(26번째 줄) 바로 아래, `RequestLogin`/`RequestSignUp` 선언 위에 추가:

```cpp
UPROPERTY(BlueprintAssignable, Category = "WebApi")
FWebApiResultSignature OnRegisterServerResult;

UPROPERTY(BlueprintAssignable, Category = "WebApi")
FWebApiServerInfoSignature OnServerInfoResult;
```

`RequestSignUp` 선언(30번째 줄) 바로 아래에 추가:

```cpp
void RequestRegisterServer(const FString& InWebServerIP, int32 InGamePort);

void RequestServerInfo(const FString& InWebServerIP);
```

`private:` 아래, 기존 `SendAuthRequest`/`HandleAuthResponse` 뒤에 추가:

```cpp
void HandleRegisterServerResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully);

void HandleServerInfoResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully);
```

- [ ] **Step 2: cpp에 구현 추가**

`Source/L20260713_Day03/Web/WebApiSubsystem.cpp`의 `RequestSignUp` 함수(24-27번째 줄) 뒤에 추가:

```cpp
void UWebApiSubsystem::RequestRegisterServer(const FString& InWebServerIP, int32 InGamePort)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("port"), InGamePort);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject, Writer);

	const FString Url = FString::Printf(TEXT("http://%s:%d/server/register"), *InWebServerIP, WebServerPort);

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Body);

	TWeakObjectPtr<UWebApiSubsystem> WeakThis(this);
	Request->OnProcessRequestComplete().BindLambda(
		[WeakThis](FHttpRequestPtr, FHttpResponsePtr InResponse, bool bInConnectedSuccessfully)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->HandleRegisterServerResponse(InResponse, bInConnectedSuccessfully);
		});

	Request->ProcessRequest();
}

void UWebApiSubsystem::RequestServerInfo(const FString& InWebServerIP)
{
	const FString Url = FString::Printf(TEXT("http://%s:%d/server/info"), *InWebServerIP, WebServerPort);

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));

	TWeakObjectPtr<UWebApiSubsystem> WeakThis(this);
	Request->OnProcessRequestComplete().BindLambda(
		[WeakThis](FHttpRequestPtr, FHttpResponsePtr InResponse, bool bInConnectedSuccessfully)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->HandleServerInfoResponse(InResponse, bInConnectedSuccessfully);
		});

	Request->ProcessRequest();
}
```

파일 끝(`HandleAuthResponse` 뒤)에 추가:

```cpp
void UWebApiSubsystem::HandleRegisterServerResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully)
{
	if (!bInConnectedSuccessfully || !InResponse.IsValid() || InResponse->GetResponseCode() != 200)
	{
		OnRegisterServerResult.Broadcast(false, TEXT("서버 등록에 실패했습니다"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InResponse->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OnRegisterServerResult.Broadcast(false, TEXT("응답을 해석할 수 없습니다"));
		return;
	}

	OnRegisterServerResult.Broadcast(JsonObject->GetBoolField(TEXT("result")), JsonObject->GetStringField(TEXT("message")));
}

void UWebApiSubsystem::HandleServerInfoResponse(FHttpResponsePtr InResponse, const bool bInConnectedSuccessfully)
{
	if (!bInConnectedSuccessfully || !InResponse.IsValid() || InResponse->GetResponseCode() != 200)
	{
		OnServerInfoResult.Broadcast(false, TEXT(""), 0, TEXT("서버 정보를 가져올 수 없습니다"));
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InResponse->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		OnServerInfoResult.Broadcast(false, TEXT(""), 0, TEXT("응답을 해석할 수 없습니다"));
		return;
	}

	if (!JsonObject->GetBoolField(TEXT("result")))
	{
		OnServerInfoResult.Broadcast(false, TEXT(""), 0, JsonObject->GetStringField(TEXT("message")));
		return;
	}

	OnServerInfoResult.Broadcast(true, JsonObject->GetStringField(TEXT("ip")), JsonObject->GetIntegerField(TEXT("port")), TEXT(""));
}
```

- [ ] **Step 3: 컴파일 확인**

Visual Studio 또는 `L20260713_Day03.sln`에서 빌드. Expected: 0 errors.

- [ ] **Step 4: Commit**

```bash
git add Source/L20260713_Day03/Web/WebApiSubsystem.h Source/L20260713_Day03/Web/WebApiSubsystem.cpp
git commit -m "feat: add server register/info requests to WebApiSubsystem"
```

---

## Task 4: `LobbyGM`에서 서버 시작 시 등록 호출

**Files:**
- Modify: `Source/L20260713_Day03/Lobby/LobbyGM.h`
- Modify: `Source/L20260713_Day03/Lobby/LobbyGM.cpp`

**Interfaces:**
- Consumes: Task 3의 `UWebApiSubsystem::RequestRegisterServer(const FString&, int32)`와 `OnRegisterServerResult`. `UDataGameInstanceSubsystem::ServerIP`(기존 필드, Title에서 로그인/회원가입 시 저장해둔 웹서버 주소).
- Produces: 없음 (로그만 남김, 다른 태스크가 의존하지 않음).

- [ ] **Step 1: 헤더에 핸들러 선언 추가**

`Source/L20260713_Day03/Lobby/LobbyGM.h`의 `BeginPlay()` 선언(29번째 줄) 아래에 추가:

```cpp
UFUNCTION()
void HandleServerRegisterResult(const bool bInSuccess, const FString& InMessage);
```

- [ ] **Step 2: cpp 상단에 상수, include 추가**

`Source/L20260713_Day03/Lobby/LobbyGM.cpp` 상단 include 목록에 추가:

```cpp
#include "../DataGameInstanceSubsystem.h"
#include "../Web/WebApiSubsystem.h"
```

`#include` 목록 바로 아래(익명 네임스페이스로 파일 스코프 상수):

```cpp
namespace
{
	constexpr int32 ListenServerPort = 7777;
}
```

- [ ] **Step 3: `BeginPlay`에서 등록 호출**

`ALobbyGM::BeginPlay()`(60-74번째 줄) 안, `Super::BeginPlay();` 바로 아래에 추가:

```cpp
if (UGameInstance* GI = GetGameInstance())
{
	if (UWebApiSubsystem* WebApi = GI->GetSubsystem<UWebApiSubsystem>())
	{
		WebApi->OnRegisterServerResult.AddUniqueDynamic(this, &ALobbyGM::HandleServerRegisterResult);

		if (UDataGameInstanceSubsystem* Data = GI->GetSubsystem<UDataGameInstanceSubsystem>())
		{
			WebApi->RequestRegisterServer(Data->ServerIP, ListenServerPort);
		}
	}
}
```

- [ ] **Step 4: 핸들러 구현**

파일 끝에 추가:

```cpp
void ALobbyGM::HandleServerRegisterResult(const bool bInSuccess, const FString& InMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("ServerRegister: %s %s"), bInSuccess ? TEXT("OK") : TEXT("FAIL"), *InMessage);
}
```

- [ ] **Step 5: 컴파일 확인**

빌드. Expected: 0 errors.

- [ ] **Step 6: PIE로 수동 검증**

`run.bat`로 웹서버 실행 → 언리얼 에디터에서 PIE로 Title 진입, 로그인 성공 후 `StartServer` 버튼으로 Lobby 진입. Output Log에서 `ServerRegister: OK` 확인. 웹서버 콘솔에도 `POST /server/register` 요청 로그가 찍혀야 한다.

- [ ] **Step 7: Commit**

```bash
git add Source/L20260713_Day03/Lobby/LobbyGM.h Source/L20260713_Day03/Lobby/LobbyGM.cpp
git commit -m "feat: register listen server IP with web server on Lobby start"
```

---

## Task 5: 로그인 후 자동 서버 조회 + `ConnectServer` 접속 대상 변경

**Files:**
- Modify: `Source/L20260713_Day03/DataGameInstanceSubsystem.h`
- Modify: `Source/L20260713_Day03/Title/TitleWidgetBase.h`
- Modify: `Source/L20260713_Day03/Title/TitleWidgetBase.cpp`

**Interfaces:**
- Consumes: Task 3의 `UWebApiSubsystem::RequestServerInfo(const FString&)`와 `OnServerInfoResult`(`bool bInSuccess, const FString& InServerIP, const int32 InServerPort, const FString& InMessage`).
- Produces: `UDataGameInstanceSubsystem::GameServerIP`(FString), `GameServerPort`(int32) — 이 태스크 안에서 쓰고 읽는다. 다른 태스크는 의존하지 않는다.

- [ ] **Step 1: `DataGameInstanceSubsystem`에 필드 추가**

`Source/L20260713_Day03/DataGameInstanceSubsystem.h`의 `ServerIP` 프로퍼티(29번째 줄) 아래에 추가:

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Data")
FString GameServerIP;

UPROPERTY(BlueprintReadOnly, Category = "Data")
int32 GameServerPort = 0;
```

- [ ] **Step 2: `TitleWidgetBase.h`에 핸들러 선언 추가**

`ProcessSignUpResult` 선언(70번째 줄) 아래에 추가:

```cpp
UFUNCTION()
void ProcessServerInfoResult(const bool bInSuccess, const FString& InServerIP, const int32 InServerPort, const FString& InMessage);
```

- [ ] **Step 3: `NativeConstruct`에서 델리게이트 바인딩 추가**

`Source/L20260713_Day03/Title/TitleWidgetBase.cpp`의 `NativeConstruct()`에서 기존:

```cpp
	if (UWebApiSubsystem* WebApi = GetWebApi())
	{
		WebApi->OnLoginResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessLoginResult);
		WebApi->OnSignUpResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessSignUpResult);
	}
```

를 아래로 교체:

```cpp
	if (UWebApiSubsystem* WebApi = GetWebApi())
	{
		WebApi->OnLoginResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessLoginResult);
		WebApi->OnSignUpResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessSignUpResult);
		WebApi->OnServerInfoResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessServerInfoResult);
	}
```

- [ ] **Step 4: 로그인 성공 시 자동 조회 + Connect 버튼 활성화 시점 변경**

`ProcessLoginResult`(148-174번째 줄) 전체를 아래로 교체:

```cpp
void UTitleWidgetBase::ProcessLoginResult(const bool bInSuccess, const FString& InMessage)
{
	bRequestInFlight = false;

	if (!bInSuccess)
	{
		SetInfoText(InMessage);
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UDataGameInstanceSubsystem* Data = GI ? GI->GetSubsystem<UDataGameInstanceSubsystem>() : nullptr;
	if (Data)
	{
		SetInfoText(FString::Printf(TEXT("%s (Lv.%d)"), *Data->Nickname, Data->Level));
	}

	if (StartServerButton)
	{
		StartServerButton->SetIsEnabled(true);
	}

	if (UWebApiSubsystem* WebApi = GetWebApi())
	{
		WebApi->RequestServerInfo(ServerIP->GetText().ToString());
	}
}
```

(`ConnectServerButton->SetIsEnabled(true)` 호출을 여기서 제거했다. 이제 서버 조회 성공 시에만 켠다.)

- [ ] **Step 5: 조회 결과 핸들러 추가**

`ProcessSignUpResult` 함수(176-181번째 줄) 뒤에 추가:

```cpp
void UTitleWidgetBase::ProcessServerInfoResult(const bool bInSuccess, const FString& InServerIP, const int32 InServerPort, const FString& InMessage)
{
	UGameInstance* GI = GetGameInstance();
	UDataGameInstanceSubsystem* Data = GI ? GI->GetSubsystem<UDataGameInstanceSubsystem>() : nullptr;

	if (!bInSuccess || !Data)
	{
		SetInfoText(InMessage);
		return;
	}

	Data->GameServerIP = InServerIP;
	Data->GameServerPort = InServerPort;

	if (ConnectServerButton)
	{
		ConnectServerButton->SetIsEnabled(true);
	}
}
```

- [ ] **Step 6: `ConnectServer()`가 조회된 서버로 접속하도록 변경**

기존 `ConnectServer()`(64-79번째 줄) 전체를 아래로 교체:

```cpp
void UTitleWidgetBase::ConnectServer()
{
	if (!IsLoggedIn())
	{
		SetInfoText(TEXT("먼저 로그인해 주세요"));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UDataGameInstanceSubsystem* Data = GI ? GI->GetSubsystem<UDataGameInstanceSubsystem>() : nullptr;
	if (!Data || Data->GameServerIP.IsEmpty())
	{
		SetInfoText(TEXT("등록된 서버가 없습니다"));
		return;
	}

	const FString Target = FString::Printf(TEXT("%s:%d"), *Data->GameServerIP, Data->GameServerPort);

	UGameplayStatics::OpenLevel(GetWorld(),
		FName(Target),
		true,
		TEXT("Key=100")
	);
}
```

(`SaveData()` 호출을 제거했다 — 더 이상 `ServerIP` 텍스트를 접속 대상으로 저장할 필요가 없다. `ServerIP` 필드 자체는 웹서버 주소로 계속 쓰인다.)

- [ ] **Step 7: 컴파일 확인**

빌드. Expected: 0 errors.

- [ ] **Step 8: PIE 2인 시나리오로 수동 검증**

`run.bat`로 웹서버 실행 (Task 1의 `game_server` 테이블이 준비된 상태).

1. PIE 인스턴스 A: 로그인 → `StartServer` 클릭 (Listen 서버로 Lobby 진입, `ServerRegister: OK` 로그 확인 — Task 4에서 이미 검증됨).
2. PIE 인스턴스 B(별도 실행): 로그인만 하고 `StartServer`는 누르지 않는다. 로그인 성공 직후 `ConnectServerButton`이 자동으로 활성화되는지 확인.
3. 인스턴스 B에서 `ConnectServerButton` 클릭 → 인스턴스 A가 등록한 IP:Port(`127.0.0.1:7777`)로 접속되어 Lobby에 함께 들어가는지 확인.

Expected: B가 A가 만든 서버에 자동으로 접속됨. 별도 IP 입력 없음.

- [ ] **Step 9: Commit**

```bash
git add Source/L20260713_Day03/DataGameInstanceSubsystem.h Source/L20260713_Day03/Title/TitleWidgetBase.h Source/L20260713_Day03/Title/TitleWidgetBase.cpp
git commit -m "feat: auto-fetch registered server IP after login and connect to it"
```
