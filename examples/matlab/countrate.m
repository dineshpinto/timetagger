host = 'http://localhost/jsonrpc/2.0/'

function json_rpc_request(method, parameter):
	url = host + ...
	buffer = urlread(url)
	return buffer
