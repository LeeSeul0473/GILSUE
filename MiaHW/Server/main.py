from fastapi import FastAPI, Request
from pydantic import BaseModel, Field
import pymysql

from db import get_connection

app = FastAPI(title="L20260713_Day03 Auth Server")


class AuthRequest(BaseModel):
    user_id: str = Field(min_length=1)
    passwd: str = Field(min_length=1)


class AuthResponse(BaseModel):
    result: bool
    message: str = ""
    idx: int = 0
    nickname: str = ""
    level: int = 0


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


@app.post("/signup", response_model=AuthResponse)
def signup(req: AuthRequest):
    conn = get_connection()
    try:
        with conn.cursor() as cur:
            try:
                cur.execute(
                    "INSERT INTO member (user_id, passwd, nickname, level)"
                    " VALUES (%s, %s, %s, 1)",
                    (req.user_id, req.passwd, req.user_id),
                )
            except pymysql.err.IntegrityError:
                return AuthResponse(result=False, message="이미 존재하는 아이디입니다")

            new_idx = cur.lastrowid

        conn.commit()
    finally:
        conn.close()

    return AuthResponse(
        result=True, idx=new_idx, nickname=req.user_id, level=1
    )


@app.post("/login", response_model=AuthResponse)
def login(req: AuthRequest):
    conn = get_connection()
    try:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT idx, nickname, level FROM member"
                " WHERE user_id = %s AND passwd = %s",
                (req.user_id, req.passwd),
            )
            row = cur.fetchone()
    finally:
        conn.close()

    if row is None:
        return AuthResponse(
            result=False, message="아이디 또는 비밀번호가 올바르지 않습니다"
        )

    return AuthResponse(
        result=True,
        idx=row["idx"],
        nickname=row["nickname"],
        level=row["level"],
    )


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
