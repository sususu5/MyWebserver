package main

import (
	"bufio"
	"encoding/binary"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"time"

	pb "termchat/build/release/proto/go"

	"google.golang.org/protobuf/proto"
)

var (
	serverAddr = flag.String("addr", "127.0.0.1:1316", "server address")
	totalMsgs  = flag.Int("n", 10000, "total messages to send")
	username   = "bench_baseline"
)

func main() {
	flag.Parse()

	fmt.Printf("=== Stage 1: Single User Benchmark ===\n")
	fmt.Printf("Target server: %s, number of messages: %d\n", *serverAddr, *totalMsgs)

	// 1. Establish connection
	conn, err := net.Dial("tcp", *serverAddr)
	if err != nil {
		log.Fatalf("Connection failed: %v", err)
	}
	defer conn.Close()

	// 2. Register and login
	rw := bufio.NewReadWriter(bufio.NewReader(conn), bufio.NewWriter(conn))
	if err := doHandshake(rw, username); err != nil {
		log.Fatalf("Handshake failed: %v", err)
	}
	fmt.Println("Logged in successfully")

	// 3. Warmup
	payload := []byte("Benchmark Payload Data")
	for i := 0; i < 100; i++ {
		if err := sendP2PMsg(rw, 0, payload); err != nil {
			log.Fatalf("Warmup send failed: %v", err)
		}
		if _, err := readResponse(rw); err != nil {
			log.Fatalf("Warmup read failed: %v", err)
		}
	}
	fmt.Println("Warmup completed")

	// 4. Send messages
	var totalDuration time.Duration
	minLatency := time.Hour
	maxLatency := time.Duration(0)

	startTime := time.Now()
	for i := 0; i < *totalMsgs; i++ {
		msgStart := time.Now()
		if err := sendP2PMsg(rw, i, payload); err != nil {
			log.Fatalf("Failed to send message at seq %d: %v", i, err)
		}
		_, err := readResponse(rw)
		if err != nil {
			log.Fatalf("Failed to read response at seq %d: %v", i, err)
		}
		latency := time.Since(msgStart)
		totalDuration += latency
		if latency < minLatency {
			minLatency = latency
		}
		if latency > maxLatency {
			maxLatency = latency
		}
	}

	totalTime := time.Since(startTime)
	avgLatency := totalDuration / time.Duration(*totalMsgs)
	qps := float64(*totalMsgs) / totalTime.Seconds()

	// 5. Print results
	fmt.Printf("\n=== Results ===\n")
	fmt.Printf("Total duration: %s\n", totalTime)
	fmt.Printf("Total messages: %d\n", *totalMsgs)
	fmt.Printf("Min latency: %s\n", minLatency)
	fmt.Printf("Max latency: %s\n", maxLatency)
	fmt.Printf("Average latency: %s\n", avgLatency)
	fmt.Printf("QPS: %.2f\n", qps)
}

func sendP2PMsg(rw *bufio.ReadWriter, seq int, content []byte) error {
	msg := &pb.P2PMessage{
		ReceiverId: 2,
		Content:    content,
		Timestamp:  time.Now().Unix(),
	}

	env := &pb.Envelope{
		Cmd: pb.CommandType_CMD_P2P_MSG_REQ,
		Payload: &pb.Envelope_P2PMsgReq{
			P2PMsgReq: msg,
		},
	}

	return sendPacket(rw, env)
}

func sendPacket(rw *bufio.ReadWriter, env *pb.Envelope) error {
	data, err := proto.Marshal(env)
	if err != nil {
		return err
	}

	lenBuf := make([]byte, 4)
	binary.BigEndian.PutUint32(lenBuf, uint32(len(data)))
	if _, err := rw.Write(lenBuf); err != nil {
		return err
	}
	if _, err := rw.Write(data); err != nil {
		return err
	}

	return rw.Flush()
}

func doHandshake(rw *bufio.ReadWriter, user string) error {
	// Register
	regReq := &pb.RegisterReq{
		Username: user,
		Password: "password",
	}
	envReq := &pb.Envelope{
		Cmd: pb.CommandType_CMD_REGISTER_REQ,
		Payload: &pb.Envelope_RegisterReq{
			RegisterReq: regReq,
		},
	}

	if err := sendPacket(rw, envReq); err != nil {
		return err
	}
	resp, err := readResponse(rw)
	if err != nil {
		return fmt.Errorf("read register response failed: %v", err)
	}
	if resp.Cmd != pb.CommandType_CMD_REGISTER_RES {
		return fmt.Errorf("register failed: expected CMD_REGISTER_RES, got %v", resp.Cmd)
	}

	// Login
	loginReq := &pb.LoginReq{
		Username: user,
		Password: "password",
	}
	envLogin := &pb.Envelope{
		Cmd: pb.CommandType_CMD_LOGIN_REQ,
		Payload: &pb.Envelope_LoginReq{
			LoginReq: loginReq,
		},
	}

	if err := sendPacket(rw, envLogin); err != nil {
		return err
	}
	resp, err = readResponse(rw)
	if err != nil {
		return fmt.Errorf("read login response failed: %v", err)
	}
	if resp.Cmd != pb.CommandType_CMD_LOGIN_RES {
		return fmt.Errorf("login failed: expected CMD_LOGIN_RES, got %v", resp.Cmd)
	}

	return nil
}

func readResponse(rw *bufio.ReadWriter) (*pb.Envelope, error) {
	lenBuf := make([]byte, 4)
	if _, err := io.ReadFull(rw, lenBuf); err != nil {
		return nil, fmt.Errorf("read length failed: %v", err)
	}

	msgLen := binary.BigEndian.Uint32(lenBuf)
	if msgLen > 10*1024*1024 {
		return nil, fmt.Errorf("message too large: %d", msgLen)
	}

	data := make([]byte, msgLen)
	if _, err := io.ReadFull(rw, data); err != nil {
		return nil, fmt.Errorf("read data failed: %v", err)
	}

	env := &pb.Envelope{}
	if err := proto.Unmarshal(data, env); err != nil {
		return nil, fmt.Errorf("unmarshal failed: %v", err)
	}
	return env, nil
}
