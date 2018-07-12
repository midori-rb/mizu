Gem::Specification.new do |s|
  s.name                     = 'mizu'
  s.version                  = '0.0.1'
  s.required_ruby_version    = '>=2.3.6'
  s.platform                 = Gem::Platform::RUBY
  s.date                     = Time.now.strftime('%Y-%m-%d')
  s.summary                  = 'Mizu is a Ruby port of h2o/picohttpparser'
  s.description              = 'Mizu is a Ruby port of h2o/picohttpparser, designed to be merged in next generation of midori.'
  s.authors                  = ['HeckPsi Lab']
  s.email                    = ['business@heckpsi.com']
  s.require_paths            = ['lib']
  s.files                    = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|.resources)/}) } \
    - %w(README.md CODE_OF_CONDUCT.md CONTRIBUTING.md Gemfile Rakefile mizu.gemspec .gitignore .codeclimate.yml .rubocop.yml .travis.yml logo.png Rakefile Gemfile)
  # s.extensions               = ['ext/mizu/extconf.rb']
  s.homepage                 = 'https://github.com/midori-rb/mizu'
  s.metadata                 = { 'issue_tracker' => 'https://github.com/midori-rb/mizu/issues' }
  s.license                  = 'MIT'
  s.add_development_dependency 'rake-compiler', '~> 1.0'
end
