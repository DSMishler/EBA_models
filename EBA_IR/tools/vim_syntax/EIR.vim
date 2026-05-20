
syn keyword EIROP       MEMOP BUFREQ INVOKE MATHOP CMP PRINT LOG SCAFFOLD
syn keyword IDENTIFIER  ALLOC RELEASE BYTES LOAD_LITERAL TRANSFER_WITH_OFFSET
syn keyword IDENTIFIER  READ_FROMBUF WRITE_TOBUF MOVE LOCAL
syn keyword IDENTIFIER  ADD_U64 SUB_U64 MUL_U64 DIV_U64 MOD_U64
syn keyword IDENTIFIER  GT GEQ EQ NEQ LT LEQ
syn keyword IDENTIFIER  STRING VAR WORD RESET
syn keyword IDENTIFIER  SYSTEM CODEREAD CODEFREE TERMINATE_WITH_CODEFREE


syn match LITERAL       "@\d\+"
syn match VAR           "V\d\+"
syn match LITBUF        "&\d\+"

syn region COMMENT      start="\/\/" end="\n"
syn region COMMENT      start="#" end="\n"


command -nargs=+ HiLink hi def link <args>

HiLink EIROP            Keyword
HiLink IDENTIFIER       Identifier
HiLink COMMENT          Comment
HiLink LITERAL          Constant
HiLink VAR              Type
HiLink LITBUF           Constant

delcommand HiLink
