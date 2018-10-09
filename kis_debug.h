#ifndef KIS_DEBUG_H
#define KIS_DEBUG_H

#include <QDebug>

/**
 * Please pretty print my variable
 *
 * Use this macro to display in the output stream the name of a variable followed by its value.
 */
#define ppVar( var ) #var << "=" << (var)

#ifdef __GNUC__
class QString;
QString __methodName(const char *prettyFunction);
#define __METHOD_NAME__ __methodName(__PRETTY_FUNCTION__)
#else
#define __METHOD_NAME__ "<unknown>:<unknown>"
#endif

#define PREPEND_METHOD(msg) QString("%1: %2").arg(__METHOD_NAME__).arg(msg)

#ifdef __GNUC__
#define ENTER_FUNCTION() qDebug() << "Entering" << __METHOD_NAME__
#define LEAVE_FUNCTION() qDebug() << "Leaving " << __METHOD_NAME__
#else
#define ENTER_FUNCTION() qDebug() << "Entering" << "<unknown>"
#define LEAVE_FUNCTION() qDebug() << "Leaving " << "<unknown>"
#endif


#endif // KIS_DEBUG_H
