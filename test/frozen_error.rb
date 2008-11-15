unless defined? FrozenError
  if "1.9.0" <= RUBY_VERSION
    FrozenError = RuntimeError
  else
    FrozenError = TypeError
  end
end
