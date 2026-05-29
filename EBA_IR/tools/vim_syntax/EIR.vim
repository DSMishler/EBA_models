
syn keyword EIROP       MEMOP BUFREQ INVOKE MATHOP CMP PRINT LOG SCAFFOLD
syn keyword IDENTIFIER  ALLOC RELEASE BYTES LOAD_LITERAL TRANSFER_WITH_OFFSET
syn keyword IDENTIFIER  READ_FROMBUF WRITE_TOBUF MOVE LOCAL
syn keyword IDENTIFIER  ADD_U64 SUB_U64 MUL_U64 DIV_U64 MOD_U64
syn keyword IDENTIFIER  GT GEQ EQ NEQ LT LEQ
syn keyword IDENTIFIER  STRING VAR WORD RESET
syn keyword IDENTIFIER  SYSTEM CODEREAD CODEFREE TERMINATE_WITH_CODEFREE
syn keyword IDENTIFIER  P_LOCK_INIT P_LOCK_FREE P_LOCK_ACK P_LOCK_REL
syn keyword IDENTIFIER  P_SEM_INIT P_SEM_FREE P_SEM_WAIT P_SEM_POST
syn keyword IDENTIFIER  PTHREAD_SPAWN_HEAVY PTHREAD_GET_AVAIL PTHREAD_GET_TID


syn match LITERAL       "@\d\+"
syn match VAR           "V\d\+"
syn match LITBUF        "&\d\+"

syn region COMMENT      start="\/\/" end="\n"
syn region COMMENT      start="#" end="\n"
syn region STRING       start="\"" end="\""


command -nargs=+ HiLink hi def link <args>

HiLink EIROP            Keyword
HiLink IDENTIFIER       Identifier
HiLink COMMENT          Comment
HiLink LITERAL          Constant
HiLink STRING           Constant
HiLink VAR              Type
HiLink LITBUF           Constant

delcommand HiLink
