
## What

* HTTP listener
* Session management
* Supports upgrade to to WS

## How


### Get Accounts

#### Result

* (array)

  * `name` (string)
  * `exchange_time_utc_min` (timestamp, ns)
  * `exchange_time_utc_max` (timestamp, ns)
  * `trade_count` (integer)

Example

```json
[{
  "name": "A1",
  "exchange_time_utc_min": 123,
  "exchange_time_utc_max": 123,
  "trade_count": 123
}
]
```

#### HTTP

`GET /accounts`

#### WS

> TODO


### Get Positions

#### Result

* (array)

  * `user` (string)
  * `strategy_id` (string)
  * `account` (string)
  * `exchange` (string)
  * `symbol` (string)
  * `long_quantity` (number)
  * `short_quantity` (number)
  * `exchange_time_utc` (timestamp, ns)

Example

> TODO

#### HTTP

`GET /positions`

### WS

> TODO


### Get Trades

#### Result

* (array)

  * `user` (string)
  * `strategy_id` (string)
  * `account` (string)
  * `exchange` (string)
  * `symbol` (string)
  * `side` (string)
  * `quantity` (number)
  * `price` (number)
  * `exchange_time_utc` (timestamp, ns)
  * `external_account` (string)
  * `external_order_id` (string)
  * `external_trade_id` (string)

Example

> TODO

#### HTTP

`GET /trades[?[account=(string)],[start_time=(timestamp, ns)]`

### WS

> TODO


### Put Trade(s)

#### Result

> TODO

Example

> TODO

#### HTTP

`PUT /trade[s]`

> TODO json body

### WS

> TODO


### Compress

#### Result

> TODO

Example

> TODO

#### HTTP

`PUT /compress`

> TODO json body

### WS

> TODO

### Get Funds

#### Result

* (array)

  * `account` (string)
  * `currency` (string)
  * `balance` (number)
  * `hold` (number)
  * `exchange_time_utc` (timestamp, ns)
  * `external_account` (string)

Example

> TODO

#### HTTP

`GET /funds[?[account=(string)],[currency=(string)]`

### WS

> TODO


