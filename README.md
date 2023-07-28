A template project for creating your own risk manager.

## Design

The design allows the Gateway to operate autonomously in a low-latency mode with asynchronous
input from the Risk Manager.

The Gateway operates on the basis of a "Taylor expansion" around the last received risk limits.
This will effectively remove database operations from the hot path: we can allow the Risk Manager
to be "slow".

* Risk Manager

  * Receives trade updates (including downloaded)
  * Persists trades into a databsae
  * Supports trade re-allocation
  * Supports corrections
  * Computes current positions and risk limits
  * Publishes current positions risk limits to gateway (keyed by sequence number)

* Gateway

  * Downloads trade updates
  * Receives live trade updates
  * Publishes trade updates to Risk Manager (drop-copy client)
  * Receives current positions and risk limits (keyed by sequence number)
  * Updates long/short positions based on trade updates
  * Computes risk exposure of working orders
  * Validates new risk exposure against current positions, current risk exposure and limits

![Design](/assets/images/risk_manager.svg)
