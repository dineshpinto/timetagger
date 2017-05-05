from jsonrpc import ServiceProxy

s = ServiceProxy("http://localhost:8080/json-rpc")

print s.scanDevices()
