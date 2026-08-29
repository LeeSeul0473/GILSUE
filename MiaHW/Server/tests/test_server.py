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
