// TODO: add header

#include "regexp_filter.h"

#include <QTextLayout>
#include <QLineEdit>
#include <QCoreApplication>

const QString RegExpFilter::separator_ = "|||";

RegExpFilter::RegExpFilter( QString text, enum SearchRegexpType type,
                            bool case_insensitive )
{
    if ( type == FixedString ) {
        include_.setPattern( QRegularExpression::escape( text ) );
        return;
    }
    auto parts = text.split( separator_ );
    if ( parts.size() >= 1 ) {
        includeText_ = parts.at( 0 );
    }
    if ( parts.size() >= 2 ) {
        hasSeparator_ = true;
        excludeText_ = parts.at( 1 );
    }

    auto opt = QRegularExpression::OptimizeOnFirstUsageOption
               | QRegularExpression::DontCaptureOption
               | QRegularExpression::UseUnicodePropertiesOption;
    if ( case_insensitive )
        opt |= QRegularExpression::CaseInsensitiveOption;
    include_.setPatternOptions( opt );
    exclude_.setPatternOptions( opt );
    include_.setPattern( includeText_ );
    exclude_.setPattern( excludeText_ );
}

bool RegExpFilter::hasMatch( QString str ) const
{
    return include_.match( str ).hasMatch()
           && ( exclude_.pattern() == "" || !exclude_.match( str ).hasMatch() );
}

QString RegExpFilter::errorMessage() const
{
    QString res = regExpErrorMsg( "include", include_ );
    if ( !res.isEmpty() )
        res += ", ";
    res += regExpErrorMsg( "exclude", exclude_ );
    return res;
}

QString RegExpFilter::regExpErrorMsg( QString name,
                                      const QRegularExpression &regExp )
{
    if ( regExp.isValid() )
        return "";
    QString res = name;
    auto offset = regExp.patternErrorOffset();
    if ( offset != -1 ) {
        res += "[" + QString::number( offset ) + "]";
    }
    res += ":";
    res += regExp.errorString();
    return res;
}

RegExpFilter &RegExpFilter::operator=( const RegExpFilter &other )
{
    include_ = other.include_;
    exclude_ = other.exclude_;
    return *this;
}