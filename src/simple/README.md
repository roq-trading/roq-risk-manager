# Simple

## Using

### FIX

```bash
$ ./test.sh --fix_debug=true | grep FIX\.4\.4 | sed 's/^.*\(FIX\.4\.4.*\)$/\1/'
```

```text
FIX.4.4|9=0000110|35=A|49=roq-fix-client-test|56=roq-fix-bridge|34=1|52=20230608-14:39:49.944|98=0|108=30|141=Y|789=1|553=tbom1|10=189|
FIX.4.4|9=0000087|35=A|49=roq-fix-bridge|56=roq-fix-client-test|34=1|52=20230608-14:39:49.947|98=0|108=5|10=157|
FIX.4.4|9=0000166|35=D|49=roq-fix-client-test|56=roq-fix-bridge|34=2|52=20230608-14:39:49.948|11=xxx|55=BTC-PERPETUAL|207=deribit|54=1|60=19700101-00:00:00.000|38=1|40=2|44=123.4|59=1|10=142|
FIX.4.4|9=0000149|35=F|49=roq-fix-client-test|56=roq-fix-bridge|34=3|52=20230608-14:39:49.948|41=xxx|11=yyy|55=BTC-PERPETUAL|207=deribit|54=1|60=19700101-00:00:00.000|10=119|
FIX.4.4|9=0000251|35=8|49=roq-fix-bridge|56=roq-fix-client-test|34=2|52=20230608-14:39:49.949|37=|11=xxx|17=A1-1686235189949282678|150=A|39=A|1=A1|55=BTC-PERPETUAL|207=deribit|54=1|40=2|38=1|44=123.4|15=USD|59=1|32=0|336=deribit|151=1|14=0|6=0|60=19700101-00:00:00.000|10=225|
FIX.4.4|9=0000258|35=8|49=roq-fix-bridge|56=roq-fix-client-test|34=3|52=20230608-14:39:49.949|37=|11=yyy|41=xxx|17=A1-1686235189949387020|150=6|39=6|1=A1|55=BTC-PERPETUAL|207=deribit|54=1|40=2|38=1|44=123.4|15=USD|59=1|32=0|336=deribit|151=1|14=0|6=0|60=19700101-00:00:00.000|10=212|
FIX.4.4|9=0000294|35=8|49=roq-fix-bridge|56=roq-fix-client-test|34=4|52=20230608-14:39:49.979|37=deribit-16459067402|11=yyy|41=xxx|17=A1-1686235189979934986|150=4|39=4|636=Y|1=A1|55=BTC-PERPETUAL|207=deribit|54=1|40=2|38=1|44=123.0|15=USD|59=1|32=0|336=deribit|151=1|14=0|6=0|60=20230608-14:39:49.962|58=success|10=061|
FIX.4.4|9=0000281|35=8|49=roq-fix-bridge|56=roq-fix-client-test|34=5|52=20230608-14:39:49.980|37=deribit-16459067402|11=yyy|17=A1-1686235189980937477|150=4|39=4|1=A1|55=BTC-PERPETUAL|207=deribit|54=1|40=2|38=1|44=123.0|15=USD|59=1|32=0|336=deribit|151=0|14=0|6=0|60=20230608-14:39:49.963|58=success|10=231|
FIX.4.4|9=0000096|35=1|49=roq-fix-client-test|56=roq-fix-bridge|34=4|52=20230608-14:39:50.044|112=111418534778411|10=107|
FIX.4.4|9=0000096|35=0|49=roq-fix-bridge|56=roq-fix-client-test|34=6|52=20230608-14:39:50.044|112=111418534778411|10=108|
FIX.4.4|9=0000100|35=1|49=roq-fix-bridge|56=roq-fix-client-test|34=7|52=20230608-14:39:54.948|112=roq-1-1686235194947|10=247|
FIX.4.4|9=0000100|35=0|49=roq-fix-client-test|56=roq-fix-bridge|34=5|52=20230608-14:39:54.948|112=roq-1-1686235194947|10=244|
FIX.4.4|9=0000100|35=1|49=roq-fix-bridge|56=roq-fix-client-test|34=8|52=20230608-14:40:00.048|112=roq-2-1686235200047|10=202|
FIX.4.4|9=0000100|35=0|49=roq-fix-client-test|56=roq-fix-bridge|34=6|52=20230608-14:40:00.048|112=roq-2-1686235200047|10=199|
FIX.4.4|9=0000100|35=1|49=roq-fix-bridge|56=roq-fix-client-test|34=9|52=20230608-14:40:05.048|112=roq-3-1686235205047|10=214|
FIX.4.4|9=0000100|35=0|49=roq-fix-client-test|56=roq-fix-bridge|34=7|52=20230608-14:40:05.048|112=roq-3-1686235205047|10=211|
FIX.4.4|9=0000101|35=1|49=roq-fix-bridge|56=roq-fix-client-test|34=10|52=20230608-14:40:10.148|112=roq-4-1686235210147|10=250|
FIX.4.4|9=0000100|35=0|49=roq-fix-client-test|56=roq-fix-bridge|34=8|52=20230608-14:40:10.148|112=roq-4-1686235210147|10=207|
```

### REST

```bash
$ curl http://localhost:2345/symbols
```

```text
["BTC-29DEC23","ETH_USDC-PERPETUAL","ETH-29MAR24","ETH-23JUN23","ETH-30JUN23","ETH-DERIBIT-INDEX","BTC-PERPETUAL","BTC_USDC-PERPETUAL","ETH-29DEC23","ETH-PERPETUAL","BTC-23JUN23","BTC-28JUL23","BTC-DERIBIT-INDEX","BTC-29MAR24","ETH-29SEP23","BTC-29SEP23","ETH-28JUL23","BTC-30JUN23"]
```

### WS

```bash
$ ./test_ws.py
```

```text
Using selector: EpollSelector
= connection is CONNECTING
> GET / HTTP/1.1
> Host: localhost:2345
> Upgrade: websocket
> Connection: Upgrade
> Sec-WebSocket-Key: y8uTpoTPMt9eA+40ZTB+KQ==
> Sec-WebSocket-Version: 13
> Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits
> User-Agent: Python/3.10 websockets/11.0.3
< HTTP/1.1 101 Switching Protocols
< Upgrade: websocket
< Connection: Upgrade
< Sec-WebSocket-Accept: MZQaarl/mfnHgO34zqQI4zpN+VI=
= connection is OPEN
> TEXT '{"jsonrpc": "2.0", "method": "new_order_single"...": 123.45}, "id": 1001}' [196 bytes]
< TEXT '{"jsonrpc":"2.0","result":"ok","id":1001}' [41 bytes]
{"jsonrpc":"2.0","result":"ok","id":1001}
> TEXT '{"jsonrpc": "2.0", "method": "order_cancel_requ...ERPETUAL"}, "id": 1002}' [181 bytes]
< TEXT '{"jsonrpc":"2.0","result":"ok","id":1002}' [41 bytes]
{"jsonrpc":"2.0","result":"ok","id":1002}
done
= connection is CLOSING
> CLOSE 1000 (OK) [2 bytes]
! failing connection with code 1006
= connection is CLOSED
```

As it was seen by the Deribit gateway

```bash
$ roq-dump ~/var/lib/roq/data/deribit-private.roq  | grep order
```

```text
1686972917776640669ns create_order={account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, position_effect=UNDEFINED, max_show_quantity=nan, order_type=LIMIT, time_in_force=GTC, execution_instructions=, request_template="", quantity=1, price=123.45, stop_price=nan, routing_id="test_001"}
1686972917776640669ns order_ack={stream_id=1, account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, type=CREATE_ORDER, origin=GATEWAY, status=FORWARDED, error=UNDEFINED, text="", request_id="zwAC6QMAAQAAVwOdkX9Gtest_001", external_account="", external_order_id="", routing_id="test_001", version=1, traded_quantity=0, round_trip_latency=0ns}
1686972917777867418ns cancel_order={account="A1", order_id=1001, request_template="", routing_id="test_002", version=2, conditional_on_version=1}
1686972917777867418ns order_ack={stream_id=1, account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, type=CANCEL_ORDER, origin=GATEWAY, status=FORWARDED, error=UNDEFINED, text="", request_id="0QAC6QMAAgAATwadkX9Gtest_002", external_account="", external_order_id="", routing_id="test_002", version=2, traded_quantity=0, round_trip_latency=0ns}
1686972917800814778ns order_ack={stream_id=1, account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, type=CANCEL_ORDER, origin=EXCHANGE, status=ACCEPTED, error=UNDEFINED, text="success", request_id="0QAC6QMAAgAATwadkX9Gtest_002", external_account="", external_order_id="16506050946", routing_id="test_002", version=2, traded_quantity=0, round_trip_latency=22866695ns}
1686972917800814778ns order_update={stream_id=1, account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, position_effect=UNDEFINED, max_show_quantity=1, order_type=LIMIT, time_in_force=GTC, execution_instructions=, create_time_utc=1686972917786000000ns, update_time_utc=1686972917786000000ns, external_account="", external_order_id="16506050946", client_order_id="zwAC6QMAAQAAVwOdkX9Gtest_001", status=WORKING, quantity=1, price=123, stop_price=nan, remaining_quantity=1, traded_quantity=0, average_traded_price=nan, last_traded_quantity=nan, last_traded_price=nan, last_liquidity=UNDEFINED, routing_id="test_002", max_request_version=2, max_response_version=2, max_accepted_version=2, update_type=INCREMENTAL, sending_time_utc=1686972917787000000ns, user="trader"}
1686972917806125242ns order_update={stream_id=1, account="A1", order_id=1001, exchange="deribit", symbol="BTC-PERPETUAL", side=BUY, position_effect=UNDEFINED, max_show_quantity=1, order_type=LIMIT, time_in_force=GTC, execution_instructions=, create_time_utc=1686972917786000000ns, update_time_utc=1686972917792000000ns, external_account="", external_order_id="16506050946", client_order_id="zwAC6QMAAQAAVwOdkX9Gtest_001", status=CANCELED, quantity=1, price=123, stop_price=nan, remaining_quantity=1, traded_quantity=0, average_traded_price=nan, last_traded_quantity=nan, last_traded_price=nan, last_liquidity=UNDEFINED, routing_id="test_002", max_request_version=2, max_response_version=2, max_accepted_version=2, update_type=INCREMENTAL, sending_time_utc=1686972917793000000ns, user="trader"}
```
