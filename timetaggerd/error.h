

/** standard jsonrpc errors */

static const int
	ERR_INVALID_REQUEST=-32600,
	ERR_PARSE_ERROR=-32700,
	ERR_METHOD_NOT_FOUND=-32601,
	ERR_INVALID_PARAMS=-32602,
	ERR_INTERNAL_ERROR=-32603,
	ERR_SERVER_ERROR=-32000;

static char const * const
	MSG_INVALID_REQUEST="invalid request";
static char const * const
	MSG_METHOD_NOT_FOUND="method not found";
static char const * const
	MSG_INVALID_PARAMS="invalid parameter";
static char const * const
	MSG_INTERNAL_ERROR="internal error";
static char const * const
	MSG_SERVER_ERROR="server error";


/** application specific errors */

static const int
	ERR_BAD_MODULE_CLASS=200,
	ERR_BAD_MODULE_ID=201;


static char const * const
	MSG_BAD_MODULE_CLASS="unknown module class";
static char const * const
	MSG_BAD_MODULE_ID="unknown module id";

