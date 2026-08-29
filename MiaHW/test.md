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
- 접속은 됐는데 다른 PC에서는 안 됨 → `run.bat`이 `127.0.0.1`에만 붙어서 원래 한 대에서만 되는 구조임. 다른 PC 테스트가 필요하면 `run.bat`의 uvicorn 실행줄에 `--host 0.0.0.0` 추가해야 함 (보안 관련이라 기본값으로는 안 넣어둠)
