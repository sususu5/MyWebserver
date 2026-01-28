#!/usr/bin/env bash
set -euo pipefail

SCYLLA_HOST=${SCYLLA_HOST:-scylla}
SCYLLA_PORT=${SCYLLA_PORT:-9042}

echo "[scylla] waiting for ScyllaDB..."
until docker exec scylla cqlsh "$SCYLLA_HOST" "$SCYLLA_PORT" -e "DESCRIBE KEYSPACES" > /dev/null 2>&1; do
  sleep 2
done

echo "[scylla] initializing keyspace and tables..."
docker exec -i scylla cqlsh "$SCYLLA_HOST" "$SCYLLA_PORT" <<'CQL'
CREATE KEYSPACE IF NOT EXISTS im
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS im.messages (
  conversation_id text,
  timestamp bigint,
  message_id bigint,
  sender_id bigint,
  receiver_id bigint,
  content_type int,
  content blob,
  PRIMARY KEY (conversation_id, timestamp, message_id)
) WITH CLUSTERING ORDER BY (timestamp DESC);
CQL

echo "[scylla] init done."
