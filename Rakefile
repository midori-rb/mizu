require 'rake/extensiontask'

spec = Gem::Specification.load('mizu.gemspec')
Rake::ExtensionTask.new('mizu_ext', spec) do |ext|
  ext.ext_dir = "ext/mizu"
end
