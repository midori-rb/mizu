module Mizu
  class Parser
    def <<(chuck)
      parse(chuck)
    end

    def on_complete(&block)
      @block = block
    end

    private def complete(params)
      @block.call(params)
    end
  end
end
