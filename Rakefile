require 'rake/extensiontask'
require 'rake/testtask'

task default: %i(spec)

Rake::TestTask.new do |t|
  t.test_files = FileList['test/**/test_*.rb']
end

spec = Gem::Specification.load('mizu.gemspec')
Rake::ExtensionTask.new('mizu_ext', spec) do |ext|
  ext.ext_dir = "ext/mizu"
end
