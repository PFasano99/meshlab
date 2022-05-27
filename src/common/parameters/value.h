/*****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2004-2021                                           \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/

#ifndef MESHLAB_VALUE_H
#define MESHLAB_VALUE_H

#include "../ml_document/cmesh.h"
#include <QColor>
#include <QString>
class MeshModel;
class MeshDocument;
class QDomElement;

/**
 * @brief The Value class
 *
 * Represents a generic parameter value for meshlab.
 * Specializations inherit from this class, depending on the type of the
 * value. Value class is an attribute of the RichParameter class.
 */
class Value
{
public:
	virtual ~Value() {}

	virtual bool getBool() const
	{
		assert(0);
		return bool();
	}
	virtual int getInt() const
	{
		assert(0);
		return int();
	}
	virtual Scalarm getFloat() const
	{
		assert(0);
		return Scalarm();
	}
	virtual QString getString() const
	{
		assert(0);
		return QString();
	}
	virtual Matrix44m getMatrix44f() const
	{
		assert(0);
		return Matrix44m();
	}
	virtual Point3m getPoint3f() const
	{
		assert(0);
		return Point3m();
	}
	virtual Shotm getShotf() const
	{
		assert(0);
		return Shotm();
	}
	virtual QColor getColor() const
	{
		assert(0);
		return QColor();
	}

	virtual bool isBool() const { return false; }
	virtual bool isInt() const { return false; }
	virtual bool isFloat() const { return false; }
	virtual bool isString() const { return false; }
	virtual bool isMatrix44f() const { return false; }
	virtual bool isPoint3f() const { return false; }
	virtual bool isShotf() const { return false; }
	virtual bool isColor() const { return false; }

	virtual QString typeName() const                             = 0;
	virtual void    set(const Value& p)                          = 0;
	virtual Value*  clone() const                                = 0;
	virtual bool    operator==(const Value& p) const             = 0;
	virtual void    fillToXMLElement(QDomElement& element) const = 0;
};

class BoolValue : public Value
{
public:
	BoolValue(const bool val) : pval(val) {};
	~BoolValue() {}

	inline bool       getBool() const { return pval; }
	inline bool       isBool() const { return true; }
	inline QString    typeName() const { return QString("Bool"); }
	inline void       set(const Value& p) { pval = p.getBool(); }
	inline BoolValue* clone() const { return new BoolValue(*this); }
	inline bool       operator==(const Value& p) const
	{
		if (p.isBool())
			return pval == p.getBool();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	bool pval;
};

class IntValue : public Value
{
public:
	IntValue(const int val) : pval(val) {}
	~IntValue() {}

	inline int       getInt() const { return pval; }
	inline bool      isInt() const { return true; }
	inline QString   typeName() const { return QString("Int"); }
	inline void      set(const Value& p) { pval = p.getInt(); }
	inline IntValue* clone() const { return new IntValue(*this); }
	inline bool      operator==(const Value& p) const
	{
		if (p.isInt())
			return pval == p.getInt();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	int pval;
};

class FloatValue : public Value
{
public:
	FloatValue(const float val) : pval(val) {}
	~FloatValue() {}

	inline Scalarm     getFloat() const { return pval; }
	inline bool        isFloat() const { return true; }
	inline QString     typeName() const { return QString("Float"); }
	inline void        set(const Value& p) { pval = p.getFloat(); }
	inline FloatValue* clone() const { return new FloatValue(*this); }
	inline bool        operator==(const Value& p) const
	{
		if (p.isFloat())
			return pval == p.getFloat();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	Scalarm pval;
};

class StringValue : public Value
{
public:
	StringValue(const QString& val) : pval(val) {}
	~StringValue() {}

	inline QString      getString() const { return pval; }
	inline bool         isString() const { return true; }
	inline QString      typeName() const { return QString("String"); }
	inline void         set(const Value& p) { pval = p.getString(); }
	inline StringValue* clone() const { return new StringValue(*this); }
	inline bool         operator==(const Value& p) const
	{
		if (p.isString())
			return pval == p.getString();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	QString pval;
};

class Matrix44fValue : public Value
{
public:
	Matrix44fValue(const Matrix44m& val) : pval(val) {}
	// Matrix44fValue(const vcg::Matrix44d& val) :pval(vcg::Matrix44f::Construct(val)) {}
	~Matrix44fValue() {}

	inline Matrix44m       getMatrix44f() const { return pval; }
	inline bool            isMatrix44f() const { return true; }
	inline QString         typeName() const { return QString("Matrix44f"); }
	inline void            set(const Value& p) { pval = p.getMatrix44f(); }
	inline Matrix44fValue* clone() const { return new Matrix44fValue(*this); }
	inline bool            operator==(const Value& p) const
	{
		if (p.isMatrix44f())
			return pval == p.getMatrix44f();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	Matrix44m pval;
};

class Point3fValue : public Value
{
public:
	Point3fValue(const Point3m& val) : pval(val) {}
	~Point3fValue() {}

	inline Point3m       getPoint3f() const { return pval; }
	inline bool          isPoint3f() const { return true; }
	inline QString       typeName() const { return QString("Point3f"); }
	inline void          set(const Value& p) { pval = p.getPoint3f(); }
	inline Point3fValue* clone() const { return new Point3fValue(*this); }
	inline bool          operator==(const Value& p) const
	{
		if (p.isMatrix44f())
			return pval == p.getPoint3f();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	Point3m pval;
};

class ShotfValue : public Value
{
public:
	ShotfValue(const Shotm& val) : pval(val) {}
	~ShotfValue() {}

	inline Shotm       getShotf() const { return pval; }
	inline bool        isShotf() const { return true; }
	inline QString     typeName() const { return QString("Shotf"); }
	inline void        set(const Value& p) { pval = p.getShotf(); }
	inline ShotfValue* clone() const { return new ShotfValue(*this); }
	inline bool        operator==(const Value& p) const
	{
		if (p.isShotf())
			return pval == p.getShotf();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	Shotm pval;
};

class ColorValue : public Value
{
public:
	ColorValue(QColor val) : pval(val) {}
	~ColorValue() {}

	inline QColor      getColor() const { return pval; }
	inline bool        isColor() const { return true; }
	inline QString     typeName() const { return QString("Color"); }
	inline void        set(const Value& p) { pval = p.getColor(); }
	inline ColorValue* clone() const { return new ColorValue(*this); }
	inline bool        operator==(const Value& p) const
	{
		if (p.isColor())
			return pval == p.getColor();
		else
			return false;
	}
	void fillToXMLElement(QDomElement& element) const;

private:
	QColor pval;
};

#endif //MESHLAB_VALUE_H
