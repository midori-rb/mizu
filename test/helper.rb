require 'simplecov'
require 'codeclimate-test-reporter'
require 'minitest/autorun'
require 'minitest/pride'

SimpleCov.start do
  add_filter '/test/'
end

require 'mizu'
