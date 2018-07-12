module Mizu
  class Parser
    def on_complete(&block)
      @callback = block
    end
  end
end
