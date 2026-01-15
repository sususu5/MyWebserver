import socket
import struct
import sys
import os
import time

current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, '..'))

# Potential paths for generated protobuf files
proto_paths = [
    os.path.join(project_root, 'proto'),                    # Source dir (legacy/manual)
    os.path.join(project_root, 'build', 'proto'),           # Simple build dir
    os.path.join(project_root, 'build', 'debug', 'proto'),  # Debug build dir
    os.path.join(project_root, 'build', 'release', 'proto') # Release build dir
]

for p in proto_paths:
    if os.path.isdir(p):
        sys.path.append(p)

def run_test(username="test_user", password="password123"):
    try:
        import message_pb2
        import auth_service_pb2
    except ImportError:
        print("[!] Protobuf modules not found. Did you run 'protoc -I=proto --python_out=proto proto/*.proto'?")
        return

    host = '127.0.0.1'
    port = 1316

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print(f"[*] Connected to {host}:{port}")
    except ConnectionRefusedError:
        print(f"[!] Connection failed. Is the server running on port {port}?")
        return

    envelope = message_pb2.Envelope()
    envelope.seq = int(time.time() * 1000)
    envelope.cmd = message_pb2.CMD_REGISTER_REQ
    envelope.timestamp = int(time.time())

    register_req = envelope.register_req
    register_req.username = username
    register_req.password = password

    serialized_data = envelope.SerializeToString()
    msg_len = len(serialized_data)

    header = struct.pack('>I', msg_len)
    sock.sendall(header + serialized_data)
    print(f"[*] Sent RegisterReq: username={username}, len={msg_len}")

    try:
        header_data = sock.recv(4)
        if len(header_data) < 4:
            print("[!] Failed to read response header")
            return
        
        resp_len = struct.unpack('>I', header_data)[0]
        
        resp_data = b''
        while len(resp_data) < resp_len:
            packet = sock.recv(resp_len - len(resp_data))
            if not packet:
                break
            resp_data += packet
            
        resp_envelope = message_pb2.Envelope()
        resp_envelope.ParseFromString(resp_data)
        
        print(f"[*] Received Response: CMD={resp_envelope.cmd}")
        
        if resp_envelope.cmd == message_pb2.CMD_REGISTER_RES:
            res = resp_envelope.register_res
            print(f"    Success: {res.success}")
            if res.success:
                print(f"    User ID: {res.user_id}")
            else:
                print(f"    Error Msg: {res.error_msg}")
        else:
            print(f"    Unexpected command: {resp_envelope.cmd}")
            
    except Exception as e:
        print(f"[!] Error: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    uname = sys.argv[1] if len(sys.argv) > 1 else "test_user_" + str(int(time.time()) % 1000)
    pwd = sys.argv[2] if len(sys.argv) > 2 else "password123"
    run_test(uname, pwd)
