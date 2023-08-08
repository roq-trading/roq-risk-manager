
## What

* HTTP listener
* Session management
* Supports upgrade to to WS

## Get Accounts

### Result

* (array)

  * `name` (string)
  * `create_time_utc_min` (timestamp, ns)
  * `create_time_utc_max` (timestamp, ns)
  * `trade_count` (integer)

Example

```json
[{
  "name": "A1",
  "create_time_utc_min": 123,
  "create_time_utc_max": 123,
  "trade_count": 123
}
]
```

### HTTP

`GET /accounts`

### WS

> TODO
