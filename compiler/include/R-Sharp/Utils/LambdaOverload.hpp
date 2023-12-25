#pragma once

template <class... Ts>
struct lambda_overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
lambda_overload(Ts...) -> lambda_overload<Ts...>;