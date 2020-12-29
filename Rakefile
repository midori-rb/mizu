begin
  require 'bundler/setup'
rescue LoadError
  puts 'You must `gem install bundler` and `bundle install` to run rake tasks'
end

require 'rake/extensiontask'
require 'rake/testtask'
require File.join('bundler', 'gem_tasks')

task default: %i(spec)

Rake::TestTask.new do |t|
  t.test_files = FileList['test/**/test_*.rb']
end

spec = Gem::Specification.load('mizu.gemspec')
Rake::ExtensionTask.new('mizu_ext', spec) do |ext|
  ext.ext_dir = "ext/mizu"
end
