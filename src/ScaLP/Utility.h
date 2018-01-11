namespace ScaLP
{
  // sum up Variables or Terms in a traversable container.
  // Tested with:
  //   - std::list<ScaLP::Variable>
  //   - std::vector<ScaLP::Variable>
  //   - std::set<ScaLP::Variable>
  //   - std::list<ScaLP::Term>
  //   - std::vector<ScaLP::Term>
  template<class C> ScaLP::Term sum(const C &c);
}

template<class C> ScaLP::Term ScaLP::sum(const C &c)
{
  ScaLP::Term t;
  for(auto e:c) t+=e;
  return t;
}
