module Mizu
  class Exceptions
    class ParseError < IOError; end
    class HeadersTooLongError < IOError; end
  end
end
