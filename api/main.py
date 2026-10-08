from datetime import datetime, timedelta, timezone
import math
import os
import random
import sqlite3
from pathlib import Path

import jwt
from fastapi import FastAPI, Header, HTTPException
from pydantic import BaseModel
from pwdlib import PasswordHash

DATABASE = Path(__file__).parent / "app.db"
SECRET_KEY = os.getenv(
    "MYIMGUI_JWT_SECRET",
    "development-only-change-this-secret",
)
ALGORITHM = "HS256"
TOKEN_HOURS = 8

password_hash = PasswordHash.recommended()
app = FastAPI(title="MyImGuiApp API", version="1.0.0")


def get_connection():
    connection = sqlite3.connect(DATABASE)
    connection.row_factory = sqlite3.Row
    return connection


def seed_stock_data(connection, symbol: str):
    random.seed(42)
    start = datetime(2026, 10, 7, 9, 30)
    price = 225.10
    rows = []

    for index in range(78):
        timestamp = start + timedelta(minutes=index * 5)
        open_price = price
        drift = 0.03 + math.sin(index / 8.0) * 0.08
        noise = random.uniform(-0.32, 0.32)
        close_price = max(1.0, open_price + drift + noise)
        high_price = max(open_price, close_price) + random.uniform(0.04, 0.28)
        low_price = min(open_price, close_price) - random.uniform(0.04, 0.28)
        volume = random.randint(120_000, 650_000)

        rows.append((
            symbol,
            timestamp.isoformat(timespec="seconds"),
            round(open_price, 4),
            round(high_price, 4),
            round(low_price, 4),
            round(close_price, 4),
            volume,
        ))
        price = close_price

    connection.executemany(
        """
        INSERT INTO stock_prices
        (symbol, timestamp, open, high, low, close, volume)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
        rows,
    )


def initialize_database():
    connection = get_connection()

    connection.execute("""
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL
        )
    """)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            value REAL NOT NULL
        )
    """)

    connection.execute("""
        CREATE TABLE IF NOT EXISTS stock_prices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            symbol TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            open REAL NOT NULL,
            high REAL NOT NULL,
            low REAL NOT NULL,
            close REAL NOT NULL,
            volume INTEGER NOT NULL
        )
    """)

    admin = connection.execute(
        "SELECT id FROM users WHERE username = ?",
        ("admin",),
    ).fetchone()

    if admin is None:
        connection.execute(
            """
            INSERT INTO users (username, password_hash)
            VALUES (?, ?)
            """,
            ("admin", password_hash.hash("password")),
        )

    if connection.execute(
        "SELECT COUNT(*) AS count FROM data"
    ).fetchone()["count"] == 0:
        connection.executemany(
            "INSERT INTO data (name, value) VALUES (?, ?)",
            [
                ("Temperature", 23.5),
                ("Pressure", 1013.2),
                ("Humidity", 45.0),
            ],
        )

    if connection.execute(
        """
        SELECT COUNT(*) AS count
        FROM stock_prices
        WHERE symbol = ?
        """,
        ("AAPL",),
    ).fetchone()["count"] == 0:
        seed_stock_data(connection, "AAPL")

    connection.commit()
    connection.close()


class LoginRequest(BaseModel):
    username: str
    password: str


class DataRequest(BaseModel):
    name: str
    value: float


class StockPriceRequest(BaseModel):
    timestamp: str
    open: float
    high: float
    low: float
    close: float
    volume: int = 0


def create_token(username: str) -> str:
    expires = datetime.now(timezone.utc) + timedelta(hours=TOKEN_HOURS)
    return jwt.encode(
        {"sub": username, "exp": expires},
        SECRET_KEY,
        algorithm=ALGORITHM,
    )


def verify_token(authorization: str | None) -> str:
    if not authorization:
        raise HTTPException(401, "Authentication required")
    if not authorization.startswith("Bearer "):
        raise HTTPException(401, "Invalid authentication header")

    try:
        payload = jwt.decode(
            authorization[7:].strip(),
            SECRET_KEY,
            algorithms=[ALGORITHM],
        )
        username = payload.get("sub")
        if not username:
            raise HTTPException(401, "Invalid token")
        return username
    except jwt.ExpiredSignatureError:
        raise HTTPException(401, "Token expired")
    except jwt.InvalidTokenError:
        raise HTTPException(401, "Invalid token")


@app.on_event("startup")
def startup():
    initialize_database()


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/login")
def login(request: LoginRequest):
    connection = get_connection()
    user = connection.execute(
        """
        SELECT username, password_hash
        FROM users
        WHERE username = ?
        """,
        (request.username,),
    ).fetchone()
    connection.close()

    if user is None or not password_hash.verify(
        request.password,
        user["password_hash"],
    ):
        raise HTTPException(401, "Invalid username or password")

    return {
        "token": create_token(user["username"]),
        "token_type": "bearer",
        "expires_in": TOKEN_HOURS * 3600,
    }


@app.get("/data")
def get_data(authorization: str | None = Header(default=None)):
    verify_token(authorization)
    connection = get_connection()
    rows = connection.execute(
        "SELECT id, name, value FROM data ORDER BY id"
    ).fetchall()
    connection.close()
    return [dict(row) for row in rows]


@app.post("/data")
def create_data(
    request: DataRequest,
    authorization: str | None = Header(default=None),
):
    verify_token(authorization)
    connection = get_connection()
    cursor = connection.execute(
        "INSERT INTO data (name, value) VALUES (?, ?)",
        (request.name, request.value),
    )
    connection.commit()
    result = {
        "id": cursor.lastrowid,
        "name": request.name,
        "value": request.value,
    }
    connection.close()
    return result


@app.put("/data/{data_id}")
def update_data(
    data_id: int,
    request: DataRequest,
    authorization: str | None = Header(default=None),
):
    verify_token(authorization)
    connection = get_connection()
    cursor = connection.execute(
        """
        UPDATE data
        SET name = ?, value = ?
        WHERE id = ?
        """,
        (request.name, request.value, data_id),
    )
    connection.commit()
    connection.close()

    if cursor.rowcount == 0:
        raise HTTPException(404, "Data record not found")

    return {
        "id": data_id,
        "name": request.name,
        "value": request.value,
    }


@app.delete("/data/{data_id}")
def delete_data(
    data_id: int,
    authorization: str | None = Header(default=None),
):
    verify_token(authorization)
    connection = get_connection()
    cursor = connection.execute(
        "DELETE FROM data WHERE id = ?",
        (data_id,),
    )
    connection.commit()
    connection.close()

    if cursor.rowcount == 0:
        raise HTTPException(404, "Data record not found")

    return {"deleted": True, "id": data_id}


@app.get("/stocks/{symbol}/prices")
def get_stock_prices(
    symbol: str,
    authorization: str | None = Header(default=None),
):
    verify_token(authorization)
    symbol = symbol.upper().strip()

    if not symbol:
        raise HTTPException(400, "Stock symbol is required")

    connection = get_connection()
    rows = connection.execute(
        """
        SELECT timestamp, open, high, low, close, volume
        FROM stock_prices
        WHERE symbol = ?
        ORDER BY timestamp
        """,
        (symbol,),
    ).fetchall()
    connection.close()

    if not rows:
        raise HTTPException(
            404,
            f"No stock data found for {symbol}",
        )

    return {
        "symbol": symbol,
        "prices": [dict(row) for row in rows],
    }


@app.post("/stocks/{symbol}/prices")
def add_stock_price(
    symbol: str,
    request: StockPriceRequest,
    authorization: str | None = Header(default=None),
):
    verify_token(authorization)
    symbol = symbol.upper().strip()

    if request.high < max(request.open, request.close):
        raise HTTPException(
            400,
            "High must be at least open and close",
        )

    if request.low > min(request.open, request.close):
        raise HTTPException(
            400,
            "Low must be at most open and close",
        )

    connection = get_connection()
    cursor = connection.execute(
        """
        INSERT INTO stock_prices
        (symbol, timestamp, open, high, low, close, volume)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
        (
            symbol,
            request.timestamp,
            request.open,
            request.high,
            request.low,
            request.close,
            request.volume,
        ),
    )
    connection.commit()
    result = {
        "id": cursor.lastrowid,
        "symbol": symbol,
        "timestamp": request.timestamp,
        "open": request.open,
        "high": request.high,
        "low": request.low,
        "close": request.close,
        "volume": request.volume,
    }
    connection.close()
    return result
