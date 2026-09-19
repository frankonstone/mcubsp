struct base {
    virtual ~base() = default;
};

struct derived final : base {};

derived* forbidden_rtti_probe(base* value) {
    return dynamic_cast<derived*>(value);
}
