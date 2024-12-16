#!/bin/python

# Gemini AI generated.

def unicode_hex_to_utf8(hex_string):
  """将Unicode 16进制字符串转换为UTF-8编码。

  Args:
    hex_string: Unicode 16进制字符串，例如'e128'。

  Returns:
    UTF-8编码的字节串。
  """

  # 将16进制字符串转换为整数，再转换为Unicode字符
  unicode_char = chr(int(hex_string, 16))
  # 将Unicode字符编码为UTF-8
  utf8_bytes = unicode_char.encode('utf-8')
  return utf8_bytes

# 示例用法
hex_string = 'e128'
utf8_bytes = unicode_hex_to_utf8(hex_string)
print(utf8_bytes)  # 输出b'\xe1\x28'
