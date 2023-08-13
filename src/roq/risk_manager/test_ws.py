#!/usr/bin/env python

import asyncio
import logging
import json
import websockets


logging.basicConfig(
    format="%(message)s",
    level=logging.DEBUG,
)


async def dispatch(ws):
    while True:
        message = await ws.recv()
        response_or_notification = json.loads(message)
        print(response_or_notification)


async def main():
    async with websockets.connect("ws://localhost:1234") as ws:
        try:
            await dispatch(ws)
        except websockets.ConnectionClosedOK:
            print("closed ok")
        except websockets.ConnectionClosedError:
            print("closed error")
        except websockets.ConnectionClosed:
            print("closed")
        print("done")


if __name__ == "__main__":
    asyncio.run(main())
