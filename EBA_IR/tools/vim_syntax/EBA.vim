
syn keyword EBAOP       MEMOP BUFREQ INVOKE LITERAL PRINT
syn keyword IDENTIFIER  ALLOC RELEASE BYTES LOAD_LITERAL TRANSFER_WITH_OFFSET
syn keyword IDENTIFIER  READ_FROMBUF WRITE_TOBUF
syn keyword IDENTIFIER  ADD_U64 SUB_U64 MUL_U64 DIV_U64 MOD_U64

syn region LITERAL      start="&" end=" "

syn region COMMENT      start="\/\/" end="\n"


command -nargs=+ HiLink hi def link <args>

HiLink EBAOP            Keyword
HiLink IDENTIFIER       Identifier
HiLink COMMENT          Comment
HiLink LITERAL          Constant

delcommand HiLink
