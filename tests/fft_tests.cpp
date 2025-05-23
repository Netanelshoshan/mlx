// Copyright © 2023 Apple Inc.

#include "doctest/doctest.h"

#include "mlx/mlx.h"

using namespace mlx::core;

TEST_CASE("test fft basics") {
  array x(1.0);
  CHECK_THROWS(fft::fft(x));
  CHECK_THROWS(fft::ifft(x));

  x = array({1.0});
  auto y = fft::fft(x);
  CHECK_EQ(y.dtype(), complex64);
  CHECK_EQ(y.size(), x.size());
  CHECK_EQ(y.item<complex64_t>(), complex64_t{1.0f, 0.0f});

  y = fft::ifft(x);
  CHECK_EQ(y.dtype(), complex64);
  CHECK_EQ(y.size(), x.size());
  CHECK_EQ(y.item<complex64_t>(), complex64_t{1.0f, 0.0f});

  x = array({complex64_t{1.0f, 1.0f}}, complex64);
  y = fft::fft(x);
  CHECK_EQ(y.size(), x.size());
  CHECK_EQ(y.item<complex64_t>(), complex64_t{1.0f, 1.0f});

  y = fft::ifft(x);
  CHECK_EQ(y.dtype(), complex64);
  CHECK_EQ(y.size(), x.size());
  CHECK_EQ(y.item<complex64_t>(), complex64_t{1.0f, 1.0f});

  {
    x = array({0.0f, 1.0f, 2.0f, 3.0f});
    y = fft::fft(x);
    std::initializer_list<complex64_t> expected = {
        {6.0, 0.0},
        {-2.0, 2.0},
        {-2.0, 0.0},
        {-2.0, -2.0},
    };
    CHECK_EQ(y.size(), x.size());
    CHECK(array_equal(y, array(expected)).item<bool>());

    y = fft::ifft(x);
    std::initializer_list<complex64_t> expected_inv = {
        {1.5, 0.0},
        {-0.5, -0.5},
        {-0.5, 0.0},
        {-0.5, 0.5},
    };
    CHECK(array_equal(y, array(expected_inv)).item<bool>());
  }

  {
    std::initializer_list<complex64_t> vals = {
        {1.0f, 1.0f}, {2.0f, 1.0f}, {1.0f, 2.0f}, {2.0f, 2.0f}};
    x = array(vals);
    y = fft::fft(x);
    std::initializer_list<complex64_t> expected = {
        {6.0, 6.0},
        {-1.0, -1.0},
        {-2.0, 0.0},
        {1.0, -1.0},
    };
    CHECK_EQ(y.size(), x.size());
    CHECK(array_equal(y, array(expected)).item<bool>());
    CHECK(array_equal(fft::ifft(y), x).item<bool>());
  }

  // Specify axes
  {
    x = array({0.0f, 1.0f, 2.0f, 3.0f}, {2, 2});
    std::initializer_list<complex64_t> expected_0 = {
        {2.0, 0.0},
        {4.0, 0.0},
        {-2.0, 0.0},
        {-2.0, 0.0},
    };
    y = fft::fft(x, 0);
    CHECK(array_equal(y, array(expected_0, {2, 2})).item<bool>());
    CHECK(array_equal(fft::ifft(y, 0), x).item<bool>());
    std::initializer_list<complex64_t> expected_1 = {
        {1.0, 0.0},
        {-1.0, 0.0},
        {5.0, 0.0},
        {-1.0, 0.0},
    };
    y = fft::fft(x, 1);
    CHECK(array_equal(y, array(expected_1, {2, 2})).item<bool>());
    CHECK(array_equal(fft::ifft(y, 1), x).item<bool>());
  }
}

TEST_CASE("test real ffts") {
  auto x = array({1.0});
  auto y = fft::rfft(x);
  CHECK_EQ(y.dtype(), complex64);
  CHECK_EQ(y.size(), x.size());
  CHECK_EQ(y.item<complex64_t>(), complex64_t{1.0f, 0.0f});

  {
    x = array({0.0f, 1.0f, 2.0f, 3.0f});
    y = fft::rfft(x);
    std::initializer_list<complex64_t> expected = {
        {6.0, 0.0}, {-2.0, 2.0}, {-2.0, -0.0}};
    CHECK_EQ(y.size(), x.size() / 2 + 1);
    CHECK(array_equal(y, array(expected)).item<bool>());
  }

  x = array(complex64_t{1, 1});
  CHECK_THROWS(fft::irfft(x));

  x = array({complex64_t{0, 1}, complex64_t{1, 0}});
  y = fft::irfft(x);
  CHECK_EQ(y.size(), 2);
  CHECK_EQ(y.dtype(), float32);
  CHECK(array_equal(y, array({0.5f, -0.5f})).item<bool>());
}

TEST_CASE("test fftn") {
  auto x = zeros({5, 5, 5});
  CHECK_THROWS_AS(fft::fftn(x, {}, {0, 3}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftn(x, {}, {0, -4}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftn(x, {}, {0, 0}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftn(x, {5, 5, 5}, {0}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftn(x, {0}, {}, {}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftn(x, {1, -1}, {}, {}), std::invalid_argument);

  // Test 2D FFT
  {
    x = array({0.0f, 1.0f, 2.0f, 3.0f}, {2, 2});
    std::initializer_list<complex64_t> expected = {
        {6.0, 0.0},
        {-2.0, 0.0},
        {-4.0, 0.0},
        {0.0, 0.0},
    };
    auto y = fft::fft2(x);
    CHECK(array_equal(y, array(expected, {2, 2})).item<bool>());
    CHECK(array_equal(fft::ifft2(y), x).item<bool>());
  }

  // Test 3D FFT
  {
    x = reshape(arange(8, float32), {2, 2, 2});
    std::initializer_list<complex64_t> expected = {
        {28.0, 0.0},
        {-4.0, 0.0},
        {-8.0, 0.0},
        {0.0, 0.0},
        {-16.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
        {0.0, 0.0},
    };
    auto y = fft::fftn(x);
    CHECK(array_equal(y, array(expected, {2, 2, 2})).item<bool>());
    CHECK(array_equal(fft::ifftn(y), x).item<bool>());

    x = reshape(arange(20, float32), {5, 4});
    y = fft::rfftn(x);
    CHECK_EQ(y.shape(), Shape{5, 3});
    y = fft::rfftn(x, {1, 0});
    CHECK_EQ(y.shape(), Shape{3, 4});

    x = reshape(arange(20, float32), {5, 4});
    y = fft::irfftn(x);
    CHECK_EQ(y.shape(), Shape{5, 6});
    y = fft::irfftn(x, {1, 0});
    CHECK_EQ(y.shape(), Shape{8, 4});
  }

  // Check the types of real ffts
  {
    x = zeros({5, 5}, float32);
    auto y = fft::rfft2(x);
    CHECK_EQ(y.shape(), Shape{5, 3});
    CHECK_EQ(y.dtype(), complex64);

    y = fft::rfftn(x);
    CHECK_EQ(y.shape(), Shape{5, 3});
    CHECK_EQ(y.dtype(), complex64);

    x = zeros({5, 5}, complex64);
    y = fft::irfft2(x);
    CHECK_EQ(y.shape(), Shape{5, 8});
    CHECK_EQ(y.dtype(), float32);

    y = fft::irfftn(x);
    CHECK_EQ(y.shape(), Shape{5, 8});
    CHECK_EQ(y.dtype(), float32);
  }
}

TEST_CASE("test fft with provided shape") {
  auto x = ones({5, 5});

  auto y = fft::fft(x, 7, 0);
  CHECK_EQ(y.shape(), Shape{7, 5});

  y = fft::fft(x, 3, 0);
  CHECK_EQ(y.shape(), Shape{3, 5});

  y = fft::fft(x, 7, 1);
  CHECK_EQ(y.shape(), Shape{5, 7});

  y = fft::fft(x, 3, 1);
  CHECK_EQ(y.shape(), Shape{5, 3});

  y = fft::rfft(x, 7, 0);
  CHECK_EQ(y.shape(), Shape{4, 5});

  y = fft::rfft(x, 3, 0);
  CHECK_EQ(y.shape(), Shape{2, 5});

  y = fft::rfft(x, 3, 1);
  CHECK_EQ(y.shape(), Shape{5, 2});
}

TEST_CASE("test fft vmap") {
  auto fft_fn = [](array x) { return fft::fft(x); };
  auto x = reshape(arange(8), {2, 4});
  auto y = vmap(fft_fn)(x);
  CHECK(array_equal(y, fft::fft(x)).item<bool>());

  y = vmap(fft_fn, 1, 1)(x);
  CHECK(array_equal(y, fft::fft(x, 0)).item<bool>());

  auto rfft_fn = [](array x) { return fft::rfft(x); };

  y = vmap(rfft_fn)(x);
  CHECK(array_equal(y, fft::rfft(x)).item<bool>());

  y = vmap(rfft_fn, 1, 1)(x);
  CHECK(array_equal(y, fft::rfft(x, 0)).item<bool>());
}

TEST_CASE("test fft grads") {
  // Regular
  auto fft_fn = [](array x) { return fft::fft(x); };
  auto cotangent = astype(arange(10), complex64);
  auto vjp_out = vjp(fft_fn, zeros_like(cotangent), cotangent).second;
  CHECK(array_equal(fft::ifft(cotangent) * 10, vjp_out).item<bool>());

  auto tangent = astype(arange(10), complex64);
  auto jvp_out = jvp(fft_fn, zeros_like(tangent), tangent).second;
  CHECK(array_equal(fft::fft(tangent), jvp_out).item<bool>());

  // Inverse
  auto ifft_fn = [](array x) { return fft::ifft(x); };
  vjp_out = vjp(ifft_fn, zeros_like(cotangent), cotangent).second;
  CHECK(array_equal(fft::fft(cotangent) * 0.1, vjp_out).item<bool>());

  jvp_out = jvp(ifft_fn, zeros_like(tangent), tangent).second;
  CHECK(array_equal(fft::ifft(tangent), jvp_out).item<bool>());

  // Real
  auto rfft_fn = [](array x) { return fft::rfft(x); };
  cotangent = astype(arange(6), complex64);
  vjp_out = vjp(rfft_fn, zeros({10}), cotangent).second;
  array mask({1.0, 0.5, 0.5, 0.5, 0.5, 1.0}, complex64);
  auto expected = fft::irfft(cotangent * mask, 10, 0) * 10;
  CHECK(array_equal(expected, vjp_out).item<bool>());

  tangent = astype(arange(10), float32);
  jvp_out = jvp(rfft_fn, zeros_like(tangent), tangent).second;
  CHECK(array_equal(fft::rfft(tangent), jvp_out).item<bool>());

  // Inverse real
  auto irfft_fn = [](array x) { return fft::irfft(x); };
  cotangent = astype(arange(10), float32);
  vjp_out = vjp(irfft_fn, astype(zeros({6}), complex64), cotangent).second;
  mask = array({0.1, 0.2, 0.2, 0.2, 0.2, 0.1}, float32);
  expected = fft::rfft(cotangent) * mask;
  CHECK(array_equal(expected, vjp_out).item<bool>());

  tangent = astype(arange(10), complex64);
  jvp_out = jvp(irfft_fn, zeros_like(tangent), tangent).second;
  CHECK(array_equal(fft::irfft(tangent), jvp_out).item<bool>());

  // Check ND vjps run properly
  vjp_out = vjp([](array x) { return fft::fftn(x); },
                astype(zeros({5, 5}), complex64),
                astype(zeros({5, 5}), complex64))
                .second;
  CHECK_EQ(vjp_out.shape(), Shape{5, 5});

  vjp_out = vjp([](array x) { return fft::ifftn(x); },
                astype(zeros({5, 5}), complex64),
                astype(zeros({5, 5}), complex64))
                .second;
  CHECK_EQ(vjp_out.shape(), Shape{5, 5});

  vjp_out = vjp([](array x) { return fft::rfftn(x); },
                zeros({5, 9}),
                astype(zeros({5, 5}), complex64))
                .second;
  CHECK_EQ(vjp_out.shape(), Shape{5, 9});

  vjp_out = vjp([](array x) { return fft::irfftn(x); },
                astype(zeros({5, 5}), complex64),
                zeros({5, 8}))
                .second;
  CHECK_EQ(vjp_out.shape(), Shape{5, 5});
}

TEST_CASE("test fftshift and ifftshift") {
  // Test 1D array with even length
  auto x = arange(8);
  auto y = fft::fftshift(x);
  CHECK_EQ(y.shape(), x.shape());
  // print y
  CHECK(array_equal(y, array({4, 5, 6, 7, 0, 1, 2, 3})).item<bool>());

  // Test 1D array with odd length
  x = arange(7);
  y = fft::fftshift(x);
  CHECK_EQ(y.shape(), x.shape());
  CHECK(array_equal(y, array({4, 5, 6, 0, 1, 2, 3})).item<bool>());

  // Test 2D array
  x = reshape(arange(16), {4, 4});
  y = fft::fftshift(x);
  auto expected =
      array({10, 11, 8, 9, 14, 15, 12, 13, 2, 3, 0, 1, 6, 7, 4, 5}, {4, 4});
  CHECK(array_equal(y, expected).item<bool>());

  // Test with specific axes
  y = fft::fftshift(x, {0});
  expected =
      array({8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7}, {4, 4});
  CHECK(array_equal(y, expected).item<bool>());

  y = fft::fftshift(x, {1});
  expected =
      array({2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9, 14, 15, 12, 13}, {4, 4});
  CHECK(array_equal(y, expected).item<bool>());

  // Test ifftshift (inverse operation)
  x = arange(8);
  y = fft::ifftshift(x);
  CHECK_EQ(y.shape(), x.shape());
  CHECK(array_equal(y, array({4, 5, 6, 7, 0, 1, 2, 3})).item<bool>());

  // Test ifftshift with odd length (different from fftshift)
  x = arange(7);
  y = fft::ifftshift(x);
  CHECK_EQ(y.shape(), x.shape());
  CHECK(array_equal(y, array({3, 4, 5, 6, 0, 1, 2})).item<bool>());

  // Test 2D ifftshift
  x = reshape(arange(16), {4, 4});
  y = fft::ifftshift(x);
  expected =
      array({10, 11, 8, 9, 14, 15, 12, 13, 2, 3, 0, 1, 6, 7, 4, 5}, {4, 4});
  CHECK(array_equal(y, expected).item<bool>());

  // Test error cases
  CHECK_THROWS_AS(fft::fftshift(x, {3}), std::invalid_argument);
  CHECK_THROWS_AS(fft::fftshift(x, {-5}), std::invalid_argument);
  CHECK_THROWS_AS(fft::ifftshift(x, {3}), std::invalid_argument);
  CHECK_THROWS_AS(fft::ifftshift(x, {-5}), std::invalid_argument);
}
