# -Zl options is for the generated file won't have any MSVCRT library dependencies
function(CpkExcludeMsvcrtBasic target)
  target_compile_options(${target} PRIVATE -Zl -GR- -GS- -Zi -nologo -Gs9999999)
endfunction()
function(CpkExcludeMsvcrt target)
  CpkExcludeMsvcrtBasic(${target})
  target_compile_options(${target} PRIVATE -Oi)
endfunction()
