require_relative './helper'

class TestParser < MiniTest::Test
  def setup
    @parser = Mizu::Parser.new
  end

  def test_that_it_could_parse_single_chuck_reqeust
    str = "GET /hoge HTTP/1.1\r\nHost: example.org\r\nUser-Agent: Hello/1.0\r\n\r\nTest"
    @parser.on_complete do
      assert_equal @parser.method, 'GET'
      assert_equal @parser.path, '/hoge'
      assert_equal @parser.headers['Host'], 'example.org'
      assert_equal @parser.headers['User-Agent'], 'Hello/1.0'
      assert_equal str.byteslice(@parser.offset..-1), 'Test'
    end
    @parser << str
  end

  def test_that_it_could_parse_multi_chuck_request
    str = "GET /hoge2 HTTP/1.1\r\nHost: example.org\r\nUser-Agent: "
    str2 = "Hello/1.0\r\n\r\nTest"
    @parser.on_complete do
      assert_equal @parser.method, 'GET'
      assert_equal @parser.path, '/hoge2'
      assert_equal @parser.headers['Host'], 'example.org'
      assert_equal @parser.headers['User-Agent'], 'Hello/1.0'
      assert_equal str2.byteslice(@parser.offset..-1), 'Test'
    end
    @parser << str
    @parser << str2
  end

  def test_that_it_could_parse_new_request_after_reset
    str = "GET /hoge3 HTTP/1.1\r\nHost: example.org\r\nUser-Agent: Hello/1.0\r\n\r\nTest"
    str2 = "GET /hoge4 HTTP/1.1\r\nHost: example.org\r\nUser-Agent: Hello/1.0\r\n\r\nTest"
    @parser.on_complete do
      assert_equal @parser.method, 'GET'
      assert_equal @parser.path, '/hoge3'
      assert_equal @parser.headers['Host'], 'example.org'
      assert_equal @parser.headers['User-Agent'], 'Hello/1.0'
      assert_equal str.byteslice(@parser.offset..-1), 'Test'
    end
    @parser << str
    @parser.reset!
    @parser.on_complete do
      assert_equal @parser.method, 'GET'
      assert_equal @parser.path, '/hoge4'
      assert_equal @parser.headers['Host'], 'example.org'
      assert_equal @parser.headers['User-Agent'], 'Hello/1.0'
      assert_equal str.byteslice(@parser.offset..-1), 'Test'
    end
    @parser << str2
  end

  def test_that_it_would_raise_error_when_illegal_request
    str = "GET /hoge3 SDFCECW/djsakdjaskl\r\n"
    assert_raises Mizu::Exceptions::ParseError do
      @parser << str
    end
  end

  def test_that_it_would_raise_error_when_request_too_long
    str = "GET /#{'w' * 2 ** 12}"
    assert_raises Mizu::Exceptions::HeadersTooLongError do
      @parser << str
    end
  end
end
