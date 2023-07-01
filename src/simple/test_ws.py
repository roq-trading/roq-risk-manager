#!/usr/bin/env python

import asyncio
import logging
import json
import websockets


logging.basicConfig(
    format="%(message)s",
    level=logging.DEBUG,
)

USERNAME = "trader"
PASSWORD = "secret"

EXCHANGE = "deribit"
SYMBOL = "BTC-PERPETUAL"

# globals
READY_1 = False
READY_2 = False

def create_request(method, params, id):
    request = dict(
        jsonrpc="2.0",
        method=method,
        params=params,
        id=id,
    )
    return json.dumps(request)


def check_response(message):
    response = json.loads(message)
    error = response.get("error")
    if error:
        raise RuntimeError(f"Unexpected: {error}")


async def logon(ws):
    params = dict(
        username=USERNAME,
        password=PASSWORD,
    )
    await ws.send(create_request("logon", params, 1000))
    message = await ws.recv()
    check_response(message)


async def order_mass_status_request(ws):
    params = dict(
        mass_status_req_id="test_000",
    )
    await ws.send(create_request("order_mass_status_request", params, 1001))
    # note! let the message dispatcher deal with the response


async def new_order_single(ws):
    params = dict(
        cl_ord_id="test_001",
        exchange=EXCHANGE,
        symbol=SYMBOL,
        side="BUY",
        ord_type="LIMIT",
        time_in_force="GTC",
        quantity=1.0,
        price=123.45,
    )
    await ws.send(create_request("new_order_single", params, 1002))
    # note! let the message dispatcher deal with the response


async def order_cancel_request(ws):
    params = dict(
        orig_cl_ord_id="test_001",
        cl_ord_id="test_002",
        exchange=EXCHANGE,
        symbol=SYMBOL,
    )
    await ws.send(create_request("order_cancel_request", params, 1003))
    # note! let the message dispatcher deal with the response


async def process_result(ws, result, id_):
    # TODO maybe find handler from id?
    pass


async def process_error(ws, error, id_):
    # TODO maybe find handler from id?
    raise RuntimeError(f"Unexpected: {error}")


async def business_message_reject(ws, params):
    msg_type = params["ref_msg_type"]
    ref_id = params["business_reject_ref_id"]  # could be used to look up a handler
    if msg_type == "AF":  # order mass status request
        # XXX !!! HACK !!!
        global READY_1
        if not READY_1:
            READY_1 = True
            await new_order_single(ws)
    else:
        raise RuntimeError(
            f"Unexpected: {params['business_reject_reason']} / {params['text']}"
        )


async def execution_report(ws, params):
    print(f"execution_report={params}")
    ord_status_req_id = params.get("ord_status_req_id", "")
    mass_status_req_id = params.get("mass_status_req_id", "")
    last_rpt_requested = params.get("last_rpt_requested", "")
    if ord_status_req_id and len(ord_status_req_id) > 0:
        pass
    elif mass_status_req_id and len(mass_status_req_id) > 0 and last_rpt_requested:
        # await order_cancel_request(ws)
        print("==> GOT LAST EXECUTION REPORT")
        global READY_1
        if not READY_1:
            READY_1 = True
            print("==> PLACE NEW ORDER")
            await new_order_single(ws)
    else:
        ord_status = params["ord_status"]
        if ord_status == "REJECTED":
            ord_rej_reason = params.get("ord_rej_reason")
            print(params)
            raise RuntimeError(f"Rejected: reason={ord_rej_reason}")
        if ord_status == "WORKING":
            print("==> ORDER IS WORKING")
            global READY_2
            if not READY_2:
                READY_2 = True
                print("==> CANCEL ORDER")
                await order_cancel_request(ws)


async def process_notification(ws, method, params):
    if method == "business_message_reject":
        await business_message_reject(ws, params)
    elif method == "execution_report":
        await execution_report(ws, params)
    else:
        raise RuntimeError(f"Unexpected: method='{method}'")


async def dispatch(ws):
    while True:
        message = await ws.recv()
        response_or_notification = json.loads(message)
        id_ = response_or_notification.get("id")
        if id_ is None:
            method = response_or_notification["method"]
            params = response_or_notification.get("params", {})
            await process_notification(ws, method, params)
        else:
            result = response_or_notification.get("result")
            if result is None:
                error = response_or_notification["error"]
                await process_error(ws, error, id_)
            else:
                await process_result(ws, result, id_)


async def main():
    async with websockets.connect("ws://localhost:2345") as ws:
        try:
            await logon(ws)
            # ... now we could receive interleaved notifications
            await order_mass_status_request(ws)
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
