#!/usr/bin/env bash
set -euo pipefail

SCYLLA_HOST=${SCYLLA_HOST:-scylla}
SCYLLA_PORT=${SCYLLA_PORT:-9042}

echo "[scylla] waiting for ScyllaDB..."
until docker exec scylla cqlsh "$SCYLLA_HOST" "$SCYLLA_PORT" -e "DESCRIBE KEYSPACES" > /dev/null 2>&1; do
  sleep 2
done

echo "[scylla] initializing keyspace and tables..."
docker exec scylla cqlsh "$SCYLLA_HOST" "$SCYLLA_PORT" <<'CQL'
CREATE KEYSPACE IF NOT EXISTS im
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS im.messages (
  conversation_id text,
  ts bigint,
  message_id text,
  sender_id text,
  receiver_id text,
  content_type int,
  content blob,
  PRIMARY KEY (conversation_id, ts, message_id)
) WITH CLUSTERING ORDER BY (ts DESC);
CQL

echo "[scylla] init done."
