/*
 * json_object.h
 *
 *  Created on: 24.04.2013
 *      Author: mag
 */

#ifndef JSON_OBJECT_H_
#define JSON_OBJECT_H_

#ifndef JSON_H_
#error please, dont include this file directly, use #include "json.h" instead
#endif


typedef std::vector<jsVar *> jsArray;
typedef std::map<std::string, jsVar *> jsMap;


class jsVar {
	friend class JsonReader;
	friend class JsonWriter;

public:
	static jsVar *parse(const std::string &);

protected:
	enum {
		NUL, STRING, NUMBER, ARRAY, OBJECT, BOOL
	} type;
	union {
		std::string *sval;
		//long *lval;
		bool *bval;
		double *dval;
		jsArray *array;
		jsMap *map;
	};

	static unsigned long usage, s_usage, m_usage, a_usage;
public:
	static void get_usage(double &, double &, double &, double &);

protected:
	inline jsVar() 					{ ++usage; type=NUL; }
	inline jsVar(bool *v)			{ ++usage; bval=v; type=BOOL; }
	inline jsVar(std::string *v)	{ ++usage; ++s_usage; sval=v; type=STRING; }
	inline jsVar(double *v) 		{ ++usage; dval=v; type=NUMBER; }
	inline jsVar(jsArray *v) 		{ ++usage; ++a_usage; array=v; type=ARRAY; }
	inline jsVar(jsMap *v) 			{ ++usage; ++m_usage; map=v; type=OBJECT; }


public:
	~jsVar();

	inline operator bool() {
		switch(type) {
		case NUL:		return false;
		case STRING:	return this->sval->size()>0;
		case NUMBER:	return this->dval!=0;
		case ARRAY:		return true;
		case OBJECT:	return true;
		case BOOL:		return this->bval;
		default: 		return false;
		}
	}

	inline bool isArray() const		{ return type==ARRAY; }
	inline bool isObject() const	{ return type==OBJECT; }
	inline bool isAtomic() const	{ return type!=OBJECT && type!=ARRAY; }

	jsVar *getValue(const std::string &v);
	jsVar *getValue(int idx);

	bool getStr(std::string &);
	bool getNumber(double &);
	bool getNumber(long long&);
	bool getNumber(long &);
	bool getNumber(int &);
	bool getNumber(unsigned int &);
	bool getBool(bool &);

	bool getStr(const std::string &v, std::string &);
	bool getNumber(const std::string &v, double &);
	bool getNumber(const std::string &v, long &);
	bool getNumber(const std::string &v, int &);
	bool getNumber(const std::string &v, unsigned int &);
	bool getNumber(const std::string &v, long long &);
	bool getBool(const std::string &v, bool &);

	bool getStr(int idx, std::string &);
	bool getNumber(int idx, double &);
	bool getNumber(int idx, int &);
	bool getNumber(int idx, unsigned int &);
	bool getNumber(int idx, long &);
	bool getNumber(int idx, long long &);
	bool getBool(int idx, bool &);

	std::string asString();

	size_t length();

	typedef jsMap::iterator iterator;
	iterator begin();
	iterator end();


	jsVar *clone();
	void pushback(jsVar *);

	void setValue(std::string, jsVar*);
	void setValue(std::string, const std::string &);

	void removeValue(std::string);

};


#endif /* JSON_OBJECT_H_ */
