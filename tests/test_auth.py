import socket
import struct
import sys
import os
import time

current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, '..'))

proto_paths = [
    os.path.join(project_root, 'build', 'debug', 'proto'),
    os.path.join(project_root, 'build', 'release', 'proto')
]

for p in proto_paths:
    if os.path.isdir(p):
        sys.path.append(p)

try:
    import protocol_pb2
    import auth_service_pb2
except ImportError:
    print("[!] Protobuf modules not found. Please run:")
    print("    protoc -I=proto --python_out=proto proto/*.proto")
    print("    OR ensure CMake has generated them in build/debug/proto")
    sys.exit(1)

def run_test(username="test_user", password="password123"):
    host = '127.0.0.1'
    port = 1316

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        print(f"[*] Connected to {host}:{port}")
    except ConnectionRefusedError:
        print(f"[!] Connection failed. Is the server running on port {port}?")
        return

    # --- 1. Register ---
    print("\n--- Testing Register ---")
    envelope = protocol_pb2.Envelope()
    envelope.seq = int(time.time() * 1000)
    envelope.cmd = protocol_pb2.CMD_REGISTER_REQ
    envelope.timestamp = int(time.time())

    register_req = envelope.register_req
    register_req.username = username
    register_req.password = password

    send_msg(sock, envelope)
    
    resp = recv_msg(sock)
    if not resp:
        return

    if resp.cmd == protocol_pb2.CMD_REGISTER_RES:
        res = resp.register_res
        if res.success:
            print(f"✅ Register Success: UserID={res.user_id}")
        else:
            print(f"❌ Register Failed: {res.error_msg}")
            if "already exists" not in res.error_msg:
                 sock.close()
                 return
    else:
        print(f"⚠️ Unexpected command: {resp.cmd}")

    # --- 2. Login ---
    print("\n--- Testing Login ---")
    envelope = protocol_pb2.Envelope()
    envelope.seq = int(time.time() * 1000) + 1
    envelope.cmd = protocol_pb2.CMD_LOGIN_REQ
    envelope.timestamp = int(time.time())

    login_req = envelope.login_req
    login_req.username = username
    login_req.password = password

    send_msg(sock, envelope)
    
    resp = recv_msg(sock)
    if not resp:
        sock.close()
        return

    if resp.cmd == protocol_pb2.CMD_LOGIN_RES:
        res = resp.login_res
        if res.success:
            print(f"✅ Login Success!")
            print(f"   UserID: {res.user_info.user_id}")
            print(f"   Token: {res.token[:20]}...") 
        else:
            print(f"❌ Login Failed: {res.error_msg}")
    else:
        print(f"⚠️ Unexpected command: {resp.cmd}")

    sock.close()

def send_msg(sock, envelope):
    serialized_data = envelope.SerializeToString()
    msg_len = len(serialized_data)
    header = struct.pack('>I', msg_len)
    sock.sendall(header + serialized_data)

def recv_msg(sock):
    try:
        header_data = sock.recv(4)
        if len(header_data) < 4:
            print("[!] Failed to read response header")
            return None
        
        resp_len = struct.unpack('>I', header_data)[0]
        
        resp_data = b''
        while len(resp_data) < resp_len:
            packet = sock.recv(resp_len - len(resp_data))
            if not packet:
                break
            resp_data += packet
            
        envelope = protocol_pb2.Envelope()
        envelope.ParseFromString(resp_data)
        return envelope
    except Exception as e:
        print(f"[!] Error receiving: {e}")
        return None

if __name__ == "__main__":
    uname = sys.argv[1] if len(sys.argv) > 1 else "user_" + str(int(time.time()) % 10000)
    pwd = sys.argv[2] if len(sys.argv) > 2 else "password123"
    run_test(uname, pwd)
