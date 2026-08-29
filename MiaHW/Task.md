# AI로 서버와 통신하는 기능 추가 - 구현 계획

## 목표

1. 언리얼 서버(Lobby, Listen)가 시작되면 웹서버(FastAPI)에 자신의 IP/Port를 등록한다.
2. 클라이언트는 로그인에 성공하면 웹서버에서 등록된 서버 IP/Port를 받아와서, 별도 IP 입력 없이 접속할 수 있게 한다.

## 설계 결정 요약

- 등록 서버는 **단일 슬롯**(항상 최신 1개만 유지, 새 서버 시작 시 덮어씀).
- 서버 IP는 **웹서버가 HTTP 요청자 IP(`request.client.host`)로 자동 인식**한다. 언리얼 쪽에서 IP를 직접 탐지하지 않는다.
- 클라이언트는 **로그인 성공 직후 자동으로** 서버 정보를 조회한다.
- 서버 종료 시 등록 해제(unregister) 기능은 만들지 않는다(YAGNI). 다음 서버가 시작되면 덮어써진다.
- 기존 `ServerIP` 입력창은 계속 "웹서버 주소"로만 쓰인다. 게임 서버 접속 주소는 더 이상 사용자가 직접 입력하지 않는다.

---

## 1. DB 스키마

새 테이블 `game_server`를 만든다. `idx`를 1로 고정한 단일 행만 사용한다.

`Server/schema.sql` 파일로 추가(신규 파일):

```sql
CREATE TABLE IF NOT EXISTS game_server (
    idx INT PRIMARY KEY,
    ip VARCHAR(45) NOT NULL,
    port INT NOT NULL,
    registered_at DATETIME NOT NULL
);
```

이 SQL은 MySQL Workbench에서 `seul` 스키마에 수동으로 실행해야 한다(기존 `member` 테이블과 동일한 방식, 별도 마이그레이션 도구 없음).

**검증**: Workbench에서 `SELECT * FROM seul.game_server;` 실행 시 빈 테이블이 정상 조회되면 완료.

---

## 2. 웹서버 (`Server/main.py`)

### 2-1. 요청/응답 모델 추가

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

### 2-2. `POST /server/register`

- `Request` 객체에서 `request.client.host`로 호출자 IP를 얻는다.
- `game_server` 테이블에 `idx=1` 행을 upsert한다(`INSERT ... ON DUPLICATE KEY UPDATE ip=..., port=..., registered_at=NOW()`).
- 성공 시 `RegisterServerResponse(result=True)` 반환.

### 2-3. `GET /server/info`

- `game_server`에서 `idx=1` 행을 조회한다.
- 행이 없으면 `ServerInfoResponse(result=False, message="등록된 서버가 없습니다")`.
- 있으면 `ServerInfoResponse(result=True, ip=..., port=...)`.

**검증**: `run.bat`로 서버 실행 후 `curl -X POST http://127.0.0.1:8080/server/register -H "Content-Type: application/json" -d "{\"port\":7777}"` → `result:true`. 이어서 `curl http://127.0.0.1:8080/server/info` → 방금 등록한 ip/port 반환 확인.

---

## 3. 언리얼 C++ - `Source/L20260713_Day03/Web/WebApiSubsystem.h/.cpp`

### 3-1. 델리게이트 추가

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWebApiServerInfoSignature,
    const bool, bInSuccess, const FString&, InServerIP, const int32, InServerPort, const FString&, InMessage);
```

- `OnRegisterServerResult` : 기존 `FWebApiResultSignature` 재사용 (bool, message).
- `OnServerInfoResult` : 새 `FWebApiServerInfoSignature`.

### 3-2. 함수 추가

- `void RequestRegisterServer(const FString& InWebServerIP, int32 InGamePort);`
  - `POST {WebServerIP}:8080/server/register`, body `{"port": InGamePort}`.
  - 응답 JSON의 `result`/`message`를 `OnRegisterServerResult`로 브로드캐스트.
- `void RequestServerInfo(const FString& InWebServerIP);`
  - `GET {WebServerIP}:8080/server/info`.
  - 응답 JSON의 `result`/`ip`/`port`/`message`를 `OnServerInfoResult`로 브로드캐스트.

기존 `SendAuthRequest`/`HandleAuthResponse`와 같은 구조(약참조 `TWeakObjectPtr`, 람다 바인딩)를 그대로 따른다. GET 요청은 body 없이 `Request->SetVerb(TEXT("GET"))`만 다르게 처리.

**검증**: 컴파일 통과. 이후 4, 5단계에서 실제 흐름으로 검증.

---

## 4. 언리얼 C++ - `Source/L20260713_Day03/DataGameInstanceSubsystem.h`

필드 추가:

```cpp
UPROPERTY(BlueprintReadOnly, Category = "Data")
FString GameServerIP;

UPROPERTY(BlueprintReadOnly, Category = "Data")
int32 GameServerPort = 0;
```

---

## 5. 언리얼 C++ - `Source/L20260713_Day03/Lobby/LobbyGM.h/.cpp`

`BeginPlay()`(기존 존재, 서버에서만 실행됨 - GameMode는 클라이언트에 스폰되지 않음)에서:

- `GetGameInstance()->GetSubsystem<UDataGameInstanceSubsystem>()->ServerIP` (Title에서 저장해둔 웹서버 주소)를 가져온다.
- `GetGameInstance()->GetSubsystem<UWebApiSubsystem>()->RequestRegisterServer(WebServerIP, 7777)` 호출. `7777`은 언리얼 기본 리슨 포트(별도 상수로 `.cpp` 상단에 정의).
- `OnRegisterServerResult`에 로그용 핸들러 하나 바인딩(`UE_LOG`로 성공/실패만 남김, UI 노출 불필요).

**검증**: PIE에서 Listen 서버로 Lobby 진입 후 웹서버 콘솔에 `/server/register` 요청 로그가 찍히는지 확인.

---

## 6. 언리얼 C++ - `Source/L20260713_Day03/Title/TitleWidgetBase.h/.cpp`

- `NativeConstruct()`에서 `WebApi->OnServerInfoResult.AddUniqueDynamic(this, &UTitleWidgetBase::ProcessServerInfoResult)` 바인딩 추가.
- `ProcessLoginResult` 성공 분기 끝에서 `WebApi->RequestServerInfo(ServerIP->GetText().ToString())` 호출 추가. 이 시점엔 `ConnectServerButton`을 아직 활성화하지 않는다.
- 새 핸들러 `ProcessServerInfoResult(bool bInSuccess, const FString& InServerIP, int32 InServerPort, const FString& InMessage)`:
  - 성공 시 `Data->GameServerIP`/`GameServerPort` 저장, `ConnectServerButton->SetIsEnabled(true)`.
  - 실패 시 `SetInfoText(InMessage)`, Connect 버튼 비활성 유지(StartServer는 그대로 로그인 성공만으로 활성화 유지).
- `ConnectServer()` 수정: `ServerIP->GetText().ToString()` 대신
  ```cpp
  const FString Target = FString::Printf(TEXT("%s:%d"), *Data->GameServerIP, Data->GameServerPort);
  UGameplayStatics::OpenLevel(GetWorld(), FName(Target), true, TEXT("Key=100"));
  ```
  로 변경. `Data`는 `GetGameInstance()->GetSubsystem<UDataGameInstanceSubsystem>()`.
- `ClearLoginState()`에서 로그아웃/로그인 실패 시 `ConnectServerButton` 비활성화 처리는 기존 로직 유지(이미 있음).

**검증**: 인스턴스 A(리슨 서버로 Start) → 인스턴스 B(로그인만) 순서로 PIE 2인 실행. B가 로그인 성공 직후 Connect 버튼이 자동 활성화되고, 클릭 시 A가 등록한 IP:Port로 접속되는지 확인.

---

## 구현 순서

1. DB 스키마 적용 (`schema.sql` 작성 + Workbench 실행) → 검증
2. 웹서버 엔드포인트 2개 구현 → curl 검증
3. `WebApiSubsystem`에 함수/델리게이트 추가 → 컴파일 검증
4. `DataGameInstanceSubsystem` 필드 추가 → 컴파일 검증
5. `LobbyGM`에 서버 등록 호출 추가 → PIE로 등록 로그 확인
6. `TitleWidgetBase`에 자동 조회 + Connect 로직 변경 → PIE 2인 접속 시나리오로 최종 검증

## 범위 밖 (하지 않음)

- 서버 목록/다중 서버 관리
- 서버 종료 시 자동 unregister
- 언리얼 쪽에서의 IP 자동 탐지
- 소켓 통신 (기존과 동일하게 HTTP+JSON만 사용)
