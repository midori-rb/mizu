require './lib/mizu'
require 'http/parser'
require 'benchmark'

N = 1_000_000

str = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: "
str2 = "Hello/1.0\r\n\r\nTest"

a = Mizu::Parser.new

a.on_complete do
  puts a.version
  puts a.headers['Host']
  puts str2.byteslice(a.offset..-1)
end

a << str
a << str2
