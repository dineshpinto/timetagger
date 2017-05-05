#include "json.h"

#include <ctype.h>
#include <sstream>

unsigned long jsVar::usage=0;
unsigned long jsVar::s_usage=0;
unsigned long jsVar::m_usage=0;
unsigned long jsVar::a_usage=0;

void jsVar::get_usage(double &jsobj, double &jsstr, double &jsmap, double &jsarr) {
	jsobj=usage;
	jsstr=s_usage;
	jsmap=m_usage;
	jsarr=a_usage;
}

jsVar::~jsVar() {
	--usage;
	switch(type) {
	case STRING:
		delete this->sval;
		--s_usage;
		break;
	case OBJECT:
		{	for (jsMap::iterator i=this->map->begin(); i!=this->map->end(); ++i ) {
				delete i->second;
			}
			delete this->map;
			--m_usage;
		}
		break;
	case ARRAY:
		{	for (jsArray::iterator i=this->array->begin(); i!= this->array->end(); ++i) {
				delete *i;
			}
			delete this->array;
			--a_usage;
		}
		break;

	case NUMBER:
		delete this->dval;
		break;
	case BOOL:
		delete this->bval;
		break;
	case NUL:
		// nothing to do
		;
	}
}

jsVar *jsVar::getValue(const std::string &key) {
	if (type!=OBJECT) return 0;
	jsMap::iterator f=map->find(key);
	if (f==map->end())
		return new jsVar();
	return f->second;
}

jsVar *jsVar::getValue(int idx) {
	if (type!=ARRAY) return 0;
	if (idx>=int(array->size()))
		return new jsVar();
	jsVar *v=(*array)[idx];
	return v;
}

bool jsVar::getStr(std::string &str) {
	if (type==STRING) str=*sval;
	else return false;
	return true;
}
bool jsVar::getStr(const std::string &key, std::string &str) {
	jsVar *v=getValue(key);
	if (v) return v->getStr(str);
	else return false;
}
bool jsVar::getStr(int idx, std::string &str) {
	jsVar *v=getValue(idx);
	if (v) return v->getStr(str);
	else return false;
}


bool jsVar::getBool(bool &v) {
	if (type!=BOOL) return false;
	v=*bval;
	return true;
}
bool jsVar::getBool(const std::string &key, bool &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getBool(v);
	else return false;
}
bool jsVar::getBool(int idx, bool &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getBool(v);
	else return false;
}

bool jsVar::getNumber(long &v) {
	if (type!=NUMBER) return false;
	v=*dval;
	return true;
}
bool jsVar::getNumber(const std::string &key, long &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getNumber(v);
	else return false;
}
bool jsVar::getNumber(int idx, long &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getNumber(v);
	else return false;
}

bool jsVar::getNumber(int &v) {
	if (type!=NUMBER) return false;
	v=*dval;
	return true;
}
bool jsVar::getNumber(const std::string &key, int &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getNumber(v);
	else return false;
}
bool jsVar::getNumber(int idx, int &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getNumber(v);
	else return false;
}

bool jsVar::getNumber(unsigned int &v) {
	if (type!=NUMBER) return false;
	v=*dval;
	return true;
}
bool jsVar::getNumber(const std::string &key, unsigned int &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getNumber(v);
	else return false;
}
bool jsVar::getNumber(int idx, unsigned int &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getNumber(v);
	else return false;
}

bool jsVar::getNumber(long long&v) {
	if (type!=NUMBER) return false;
	v=*dval;
	return true;
}
bool jsVar::getNumber(const std::string &key, long long &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getNumber(v);
	else return false;
}
bool jsVar::getNumber(int idx, long long &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getNumber(v);
	else return false;
}

bool jsVar::getNumber(double &v) {
	if (type!=NUMBER) return false;
	v=*dval;
	return true;
}
bool jsVar::getNumber(const std::string &key, double &v) {
	jsVar *jv=getValue(key);
	if (jv) return jv->getNumber(v);
	else return false;
}
bool jsVar::getNumber(int idx, double &v) {
	jsVar *jv=getValue(idx);
	if (jv) return jv->getNumber(v);
	else return false;
}



std::string jsVar::asString() {
	if (type == STRING)
		return *(this->sval);
	std::stringstream ss;
	ss << this;
	return ss.str();
}

static jsMap dummy_map;

jsVar::iterator jsVar::begin() {
	// should rather assert or throw...
	if (isObject())
		return map->begin();
	else
		return dummy_map.end();
}

jsVar::iterator jsVar::end() {
	// should rather asert or throw...
	if (isObject())
		return map->end();
	else
		return dummy_map.end();
}

size_t jsVar::length() {
	if (isArray())
		return array->size();
	return 0;
}

jsVar *jsVar::parse(const std::string &str) {
	std::stringstream ss(str);
	jsVar *var;
	ss >> var;
	return var;
}
#include <iostream>

jsVar *jsVar::clone() {

	switch(type) {
	case OBJECT: {
		jsMap *map=new jsMap();
		for(jsVar::iterator i=begin(); i!=end(); ++i) {
			std::string key=i->first;
			(*map)[key]=i->second->clone();
		}
		return new jsVar(map);
	}
	case ARRAY: {
		jsArray *array=new jsArray;
		for (size_t n=0; n<this->length(); ++n) {
			jsVar *v=(*this->array)[n]->clone();
			array->push_back(v);
		}
		return new jsVar(array);
	}
	case NUMBER: {
		double *v=new double(*(this->dval));
		return new jsVar(v);
	}
	case STRING: {
		std::string *v=new std::string(*(this->sval));
		return new jsVar(v);
	}
	case BOOL: {
		bool *v=new bool(*(this->bval));
		return new jsVar(v);
	}
	case NUL:
	default:
		return new jsVar();
	}
}


void jsVar::pushback(jsVar *val) {
	if (isArray()) {
		this->array->push_back(val);
	}
}

void jsVar::setValue(std::string key, const std::string &value) {
	std::string *v=new std::string(value);
	setValue(key, new jsVar(v));
}

void jsVar::setValue(std::string key, jsVar *value) {
	if (isObject()) {
		removeValue(key);
		(*this->map)[key]=value;
	}
}

void jsVar::removeValue(std::string key) {
	if (isObject()) {
		jsVar *v=getValue(key);
		if (v) {
			delete v;
			this->map->erase(key);
		}
	}
}


void JsonWriter::write_string(std::ostream &out, const std::string &str) {
	out << '\"';
	for (size_t n=0; n<str.size(); ++n) {
		char ch=str.at(n);
		switch(ch) {
		case '\"':		out << "\\\""; break;
		case '\n':		out << "\\n"; break;
		case '\t':		out << "\\t"; break;
		case '\\':		out << "\\\\"; break;
		default:		out << ch;
		}
	}
	out << '\"';
}

void JsonWriter::write(std::ostream &out, jsVar *var) {
	if (!var) {
		out << "null"; return;
	}
	bool first;
	switch(var->type) {
	case jsVar::NUL:	out << "null"; break;
	case jsVar::NUMBER:	out << (*var->dval); break;
	case jsVar::BOOL:	out << (*(var->bval)? "true":"false"); break;
	case jsVar::OBJECT:	out << '{';
						first=true;
						for (jsMap::iterator i=var->map->begin(); i!=var->map->end(); ++i) {
							if (!first) out << ',';
							first=false;
							write_string(out, i->first);
							out << ": ";
							out << i->second;
						}
						out << '}';
						break;
	case jsVar::ARRAY:	out << '[';
						first=true;
						for (size_t n=0; n<var->array->size(); ++n) {
							if (!first) out << ',';
							first=false;
							out << (*(var->array))[n];
						}
						out << ']';
						break;
	case jsVar::STRING:	write_string(out, *(var->sval));
	}
}

std::istream &operator >>(std::istream &in, jsVar *&v) {
	JsonReader reader(in);
	jsVar *var=reader.parse();
	v=var;
	return in;
}

JsonReader::JsonReader(std::istream &i, MODE m) : in(i), lino(1), pos(0), mode(m) {
}

jsVar *JsonReader::parse() {
	if (mode==JSON) {
		char ch=nch_skip_blanks();
		if  (ch!='{')
			return syntax_error();
		unget();
	}
	return parse_var();
}


jsVar *JsonReader::parse_var() {
	char c;
	c=nch_skip_blanks();
	switch(c) {
	case '[':
		do {
			bool first=true;
			jsArray *array=new jsArray();
			for (;;) {
				if (first) {
					c=nch_skip_blanks();
					if (c==']') {
						break;
					} else {
						unget();
					}
				}
				first=false;

				jsVar *item=parse_var();
				if (!item) {
					delete array;
					return 0;
				}
				array->push_back(item);

				if (eof()) {
					delete array;
					return syntax_error();
				}

				c=nch_skip_blanks();
				if (c==']')
					break;
				if (c!=',') {
					delete array;
					return syntax_error();
				}
			}
			return new jsVar(array);
		} while(false);

	case '{':
		{
			jsMap *object=new jsMap();

			bool first=true;
			for  (;;) {
				c=nch_skip_blanks();
				if (first && c=='}')
					break;
				first=false;

				std::string *identifier;
				if  (c=='\"') {
					identifier=parse_string(c);
					if (!identifier) {
						delete object;
						return 0;
					}
				} else if (c=='\'') {
					if (mode==JSON) {
						delete object;
						return syntax_error();
					}
					identifier=parse_string(c);
				} else if (isalpha(c))
					identifier=parse_identifier();
				else {
					delete object;
					return syntax_error();
				}

				c=nch_skip_blanks();
				if (c!=':') {
					delete object; delete identifier;
					return syntax_error();
				}

				jsVar *value=parse_var();
				if (!value) {
					delete object;
					return 0;
				}
				(*object)[*identifier]=value;
				//delete identifier;
				if (eof()) {
					delete object;
					return syntax_error();
				}

				c=nch_skip_blanks();
				if (c=='}')
					break;
				if (c!=',') {
					delete object;
					return syntax_error();
				}
			}

			return new jsVar(object);

		}

	case '\'':
		if (mode==JSON) {
			return syntax_error();
		}
		return new jsVar(parse_string(c));
	case '\"':
		return new jsVar(parse_string(c));

	default:
		if (isdigit(c) || c=='-' || c=='+') {
			unget();
			return new jsVar(new double(parse_number()));
		}

		else if (isalpha(c)) {
			unget();
			jsVar *v=0;
			std::string *ident=parse_identifier();
			if (*ident=="null")
				v=new jsVar();
			if (*ident=="true")
				v=new jsVar(new bool(true));
			if (*ident=="false")
				v=new jsVar(new bool(false));
			delete ident;
			if (!v)
				return syntax_error();
			else
				return v;
		}

		else {
			return syntax_error();
		}
	}
}

std::string *JsonReader::parse_string(char delim) {
	char c;
	std::string *str = new std::string();
	std::string &result=*str;
	for (;;) {
		if (eof())
			syntax_error();
		c=nch();
		if (c==delim) {
			break;
		} else  if (c=='\n') {
			syntax_error();
		} else if (c=='\\') {
			c=nch();
			switch(c) {
			case 'n':	result+='\n'; break;
			case 'r':	result+='\r'; break;
			case 't':	result+='\t'; break;
			case 'b':	result+='\b'; break;
			case 'f':	result+='\f'; break;
			case '\\':	result+='\\'; break;
			case '/':	result+='/'; break;
			case '\"':	result+='\"'; break;
			case '\'':	result+='\''; break;
			case 'u': {
				unsigned uc=parse_hex();
				append_utf8(result, uc);
			}
			default:
				syntax_error();
			}
		} else {
			result+=c;
		}
	}
	return str;
}

unsigned JsonReader::parse_hex() {
	unsigned num=0;
	for (int n=0; n<4; ++n) {
		if (eof())
			syntax_error();
		char c=nch();
		if (c>='0' && c<='9') {
			num = (num<<4) | (c-'0');
		} else {
			c=toupper(c);
			if (c>='A' && c<='F') {
				num = (num<<4) | (c-'A'+10);
			} else
				syntax_error();
		}
	}
	return num;
}

void JsonReader::append_utf8(std::string &result, unsigned uc) {
	if (uc<=0x7f) {
		result += char(uc);

	} else if (uc<=0x7ff) {
		result +=char(  0xc0 | ((uc>>6)&0x1f) );
		result +=char(  0x80 | ((uc>>0)&0x3f) );

	} else if (uc<=0xffff) {
		result +=char(  0xc0 | ((uc>>12)&0x0f) );
		result +=char(  0x80 | ((uc>>6)&0x3f) );
		result +=char(  0x80 | ((uc>>0)&0x3f) );
	}
}

std::string *JsonReader::parse_identifier() {
	std::string *ident=new std::string();
	for (;;) {
		if (eof())
			break;
		char c=nch();
		if (isalnum(c)) {
			(*ident)+=c;
		} else {
			unget();
			break;
		}
	}
	return ident;
}

double JsonReader::parse_number() {
	double num=0;
	std::ios::pos_type n=in.tellg();
	in >> num;
	pos += (in.tellg()-n);
	return num;
}

char JsonReader::nch() {
	char ch;
	in.get(ch);
	++pos;
	if (ch=='\n') {
		++lino;
		pos=0;
	}
	return ch;
}

char JsonReader::nch_skip_blanks() {
	char ch=nch();
	while (isspace(ch))
		ch=nch();
	return ch;
}

bool JsonReader::eof() {
	return in.eof();
}

void JsonReader::unget() {
	in.unget();
}

jsVar *JsonReader::syntax_error() {
	std::string msg;

	msg="syntax error in JSON string.\ntext:";
	in.seekg(-15, in.cur);
	for (int n=0; n<30; ++n) {
		char c;
		in >> c;
		if (isprint(c)) msg+=c; else msg+='.';
	}
	msg+="                    ^\n";
	return 0;
}
