
/*The is the lex file for specifying the patterns*/

%{ 
	#include <stdio.h>
	#include "scan.h"
%}

keywords			(?i:allocatable)|(?i:allocate)|(?i:assign)|(?i:associate)|(?i:asynchronous)|(?i:backspace)|(?i:bind)|(?i:call)|(?i:case)|(?i:class)|(?i:close)|(?i:common)|(?i:contains)|(?i:continue)|(?i:cycle)|(?i:codimension)|(?i:contiguous)|(?i:critical)|(?i:data)|(?i:deallocate)|(?i:deferred)|(?i:dimension)|(?i:do)|(?i:else)|(?i:elsewhere)|(?i:end)|(?i:endfile)|(?i:endif)|(?i:entry)|(?i:equivalence)|(?i:exit)|(?i:external)|(?i:enum)|(?i:enumerator)|(?i:extends)|(?i:forall)|(?i:final)|(?i:flush)|(?i:format)|(?i:function)|(?i:goto)|(?i:generic)|(?i:import)|(?i:if)|(?i:implicit)|(?i:inquire)|(?i:intrinsic)|(?i:include)|(?i:interface)|(?i:intent)|(?i:lock)|(?i:module)|(?i:nopass)|(?i:namelist)|(?i:nullify)|(?i:open)|(?i:only)|(?i:operator)|(?i:optional)|(?i:parameter)|(?i:pause)|(?i:print)|(?i:program)|(?i:pointer)|(?i:private)|(?i:pass)|(?i:protected)|(?i:procedure)|(?i:public)|(?i:read)|(?i:return)|(?i:recursive)|(?i:result)|(?i:rewind)|(?i:rewrite)|(?i:save)|(?i:stop)|(?i:subroutine)|(?i:select)|(?i:sequence)|(?i:submodule)|(?i:target)|(?i:then)|(?i:use)|(?i:value)|(?i:unlock)|(?i:volatile)|(?i:while)|(?i:write)
special_chars        \\|[{]|[}]|[,]|[.]|[:]|[;]|["]|[~]|[?]|[']|[`]|\^|[|]|[$]|[#]|[@]|[!]
whitespace          [ ]|\t|\f|\n|\r
operators            ?i:"="|"+"|"-"|"*"|"/"|"<"|">"|"%"|"&"|"&&"|"<<"|">>"|"||"|"**"|"//"|".EQ."|".NE."|".LT."|".LE."|".GT."|".GE."|"=="|"/="|"<="|">="|".NOT."|".AND."|".OR."|".EQV."|".NEQV."
%option nounput yylineno

%%

{keywords}		 								return KEYWORD;

[a-zA-Z][a-zA-Z0-9_]{0,62} 						return NAME;

[+-]?[0-9][0-9]*										return INT_LITERAL;

([+-]?)(((([0-9]+[.]([0-9]+?))|([.][0-9]+))(([d-eD-E]([+-]?)[0-9]+)?))|([0-9]+[d-eD-E]([+-]?)[0-9]+))  return REAL_LITERAL;

"."(?i:true)"."											return LOGICAL_LITERAL;
"."(?i:false)"."										return LOGICAL_LITERAL;

["][^"']*["]											return CHAR_LITERAL;
["][^"]*												return STR_ERROR;
["][^"]*['][^"]*["]										return STR_ERROR;

{operators}												return OPERATOR;

"("|")"													return DELIMITER;	
"["|"]"													return DELIMITER;
"(/"|"/)"												return DELIMITER;

^[ \t]*(!.*)?											return COMMENT;

{special_chars}											return SPECIAL_CHAR;
{whitespace}											;
[ \t]*													;
.														return ERROR;

%%

int yywrap(){
	return 1;
}

