# 테스트 방법 (서버 IP 자동 등록/조회 기능)

순서대로 하면 됨: DB 준비 → 웹서버 실행 → (선택) 자동 테스트 → 언리얼 실행 → 2인 접속 확인.

## 1. DB 준비

MySQL이 이미 켜져 있고 `seul` 스키마와 `member` 테이블은 있다고 가정 (로그인/회원가입 기능이 이미 그걸 씀). 이번에 추가된 `game_server` 테이블만 새로 만들면 됨.

1. MySQL Workbench 실행 → `seul` 스키마 열기
2. 아래 SQL을 쿼리창에 붙여넣고 실행 (`Server/schema.sql` 내용 그대로)

```sql
CREATE TABLE IF NOT EXISTS game_server (
    idx INT PRIMARY KEY,
    ip VARCHAR(45) NOT NULL,
    port INT NOT NULL,
    registered_at DATETIME NOT NULL
);
```

3. `SELECT * FROM seul.game_server;` 실행했을 때 에러 없이 빈 결과 나오면 완료.

## 2. 웹서버 실행

```powershell
cd Server
python -m venv .venv
.venv\Scripts\pip install -r requirements.txt
run.bat
```

(`.venv`가 이미 있으면 첫 두 줄은 생략)

`Uvicorn running on http://127.0.0.1:8080` 이 뜨면 정상. 브라우저에서 `http://127.0.0.1:8080/docs` 열어서 `/login`, `/signup`, `/server/register`, `/server/info` 4개 엔드포인트가 보이는지 확인.

이 창은 테스트하는 동안 계속 켜둘 것.

## 3. (선택) 웹서버 자동 테스트

새 터미널에서:

```powershell
cd Server
.venv\Scripts\pytest tests\test_server.py -v
```

`4 passed`가 나오면 됨. DB 안 켜져 있어도 통과함 (테스트는 DB를 가짜로 대체해서 돌아감).

## 4. 언리얼 프로젝트 빌드/실행

1. `L20260713_Day03.uproject` 더블클릭해서 에디터 열기 (C++ 변경사항이 있으니 처음 열 때 "빌드 필요" 물어보면 예, 눌러서 빌드)
2. 빌드 끝나면 에디터에서 `Content/Maps/Title.umap` 열기

## 5. 실제 시나리오 테스트 (2인)

인스턴스 2개가 필요함 (같은 PC에서 창 2개 띄우면 됨). 에디터 툴바 Play 버튼 옆 드롭다운 → **Standalone Game** 선택.

**인스턴스 A (서버 역할):**
1. Standalone Game 실행
2. Title 화면에서 웹서버 주소 입력 (`127.0.0.1`), 아이디/비번 입력
3. 계정 없으면 회원가입 → 로그인
4. 로그인 성공하면 `StartServer` 버튼 눌러서 Lobby 진입 (Listen 서버로 시작됨)
5. Output Log에서 `ServerRegister: OK` 로그 확인 → 웹서버 터미널에도 `POST /server/register` 요청 로그 찍힘

**인스턴스 B (클라이언트 역할):**
1. Standalone Game 한 번 더 실행 (Title 창이 하나 더 뜸)
2. 웹서버 주소(`127.0.0.1`) + 다른 계정으로 로그인만 함 (StartServer는 누르지 않음)
3. 로그인 성공 직후 `ConnectServer` 버튼이 자동으로 활성화되는지 확인 (별도 IP 입력 없음)
4. `ConnectServer` 클릭 → 인스턴스 A가 만든 Lobby로 접속돼서 같이 들어가지는지 확인

성공 기준: B가 IP를 직접 입력하지 않고도 A의 서버에 자동으로 붙는 것.

## 문제 생겼을 때 체크리스트

- `ConnectServer` 버튼이 안 켜짐 → A가 `StartServer`를 먼저 눌러서 등록이 끝났는지, 웹서버 터미널에 `/server/register` 로그가 찍혔는지 확인
- 웹서버 연결 안 됨 메시지 → `run.bat` 창이 계속 켜져 있는지, 방화벽이 8080 포트를 막고 있는지 확인
- DB 에러 → `Server/db.py`의 접속 정보(호스트/포트/계정)가 실제 MySQL과 맞는지 확인
- 접속은 됐는데 다른 PC에서는 안 됨 → 아래 "다른 PC에서 테스트할 때" 항목 참고. 원인이 두 개라 한 줄로 안 끝남


## 다른 PC에서 테스트할 때 (현재 구조의 제약)

같은 PC 안에서 창 2개로 하는 테스트는 위 절차대로 하면 통과한다. **PC 2대로 넘어가면 두 가지를 손봐야 한다.**

### 1) 웹서버를 외부에 열기

`run.bat`의 uvicorn 실행줄에 `--host 0.0.0.0`을 추가한다. 기본값은 `127.0.0.1` 전용이라 다른 PC에서 8080에 닿지 못한다. 방화벽에서 8080 인바운드도 허용해야 한다.

```
.venv\Scripts\python.exe -m uvicorn main:app --host 0.0.0.0 --port 8080
```

### 2) 웹서버와 리슨서버를 같은 PC에서 돌리지 말 것

서버 등록은 웹서버가 본 요청 소스 IP(`request.client.host`)를 그대로 DB에 저장한다. 웹서버와 리슨서버(인스턴스 A)가 같은 PC면 그 값이 `127.0.0.1`로 저장되고, 다른 PC의 클라이언트 B는 `127.0.0.1`을 받아 자기 자신에게 접속을 시도해서 실패한다.

권장 배치:

| 역할 | 배치 |
|---|---|
| 웹서버(FastAPI) + MySQL | PC 1 |
| 인스턴스 A (StartServer) | PC 2 |
| 인스턴스 B (ConnectServer) | PC 3 (또는 PC 1) |

A와 B 모두 Title 화면의 웹서버 주소 칸에 **PC 1의 LAN IP**를 입력한다.

PC 2대만 있다면: 웹서버는 PC 1, 인스턴스 A는 PC 2, 인스턴스 B는 PC 1에 두면 된다. A가 등록하는 IP가 PC 2의 LAN IP가 되므로 정상 동작한다.

### 3) 게임 포트는 7777 고정

클라이언트 접속 포트는 `LobbyGM.cpp`에 7777로 하드코딩돼 있다. 언리얼을 `-port=` 옵션으로 다른 포트에 띄우면 DB에는 여전히 7777이 저장되어 B의 접속이 실패한다. 포트 옵션 없이 기본값으로 실행할 것. 방화벽에서 7777 인바운드(UDP)도 허용해야 한다.

## 재테스트 전 초기화

등록된 서버 정보는 **자동으로 지워지지 않는다.** 인스턴스 A를 껐다 다시 켜거나 다른 PC로 옮겨서 테스트하면, `/server/info`가 이전에 등록된 죽은 IP를 그대로 반환해서 B의 `ConnectServer`가 응답 없이 멈출 수 있다.

테스트 조건을 바꿀 때마다 아래를 먼저 실행한다.

```sql
DELETE FROM seul.game_server;
```

현재 등록된 값은 브라우저로 바로 확인할 수 있다.

```
http://<웹서버IP>:8080/server/info
```

`{"result":true,"ip":"...","port":7777,"message":""}` 형태로 나오고, ip가 지금 A를 돌리는 PC의 주소와 일치하는지 확인한다. `{"result":false,...}`면 아직 등록되지 않은 상태다.

## member 테이블에 대한 보충

`Server/schema.sql`에는 `game_server` 테이블만 들어 있다. 로그인/회원가입이 쓰는 `member` 테이블은 이전 작업에서 만든 것으로, 스키마 파일에 포함돼 있지 않다. 새 PC나 새 DB에서 처음부터 세팅한다면 아래 구조로 먼저 만들어야 `/login`, `/signup`이 동작한다.

```sql
CREATE TABLE IF NOT EXISTS member (
    idx INT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(45) NOT NULL,
    passwd VARCHAR(255) NOT NULL,
    nickname VARCHAR(45) NOT NULL,
    level INT NOT NULL DEFAULT 1,
    UNIQUE KEY user_id_UNIQUE (user_id)
);
```

기존 `seul` DB를 그대로 쓴다면 이 단계는 건너뛴다.
