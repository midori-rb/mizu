require './lib/mizu'

a = Mizu::Parser.new

str = "GET /hoge HTTP/1.1\r\nHost: example.com\r\nUser-Agent: 你"
str2 = "好/1.0\r\n\r\nTest"
a.on_complete do |param|
  puts param
  puts str2.byteslice(param[:offset]..-1) # Body
end
a << str
a << str2
