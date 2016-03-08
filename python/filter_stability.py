import numpy as np
from boundary_modelling import get_notch_coeffs, series_coeffs
from collections import namedtuple

Surface = namedtuple('Surface', ['specular', 'diffuse'])

def a2db(a):
    return 20 * np.log10(a)

def to_filter_coefficients(surface, sr):
    num_descriptors = 3
    edges = [40, 175, 350, 700, 1400, 2800, 5600, 11200, 20000]
    coeffs = []
    for i in range(num_descriptors):
        gain = a2db((surface.specular[i] + surface.diffuse[i]) * 0.5)
        centre = (edges[i] + edges[i + 1]) * 0.5
        coeffs.append(get_notch_coeffs(gain, centre, sr, 1.414))
    return coeffs

def is_stable_roots(polynomial):
    return np.all(np.abs(np.roots(polynomial)) < 1)

def is_stable_recursive(a):
    if len(a) == 0:
        return True

    rci = a[-1]
    if np.abs(rci) >= 1:
        return False

    next_size = len(a) - 1
    next_array = []
    for i in range(next_size):
        next_array.append((a[i] - rci * a[next_size - i]) / (1 - rci * rci))

    return is_stable_recursive(next_array)

def is_stable_jury(polynomial):
    # vvd.push_back(v);
    # reverse(v.begin(),v.end());
    # vvd.push_back(v);
    #
    # for(i=2;;i+=2)
    # {
    #     v.clear();
    #     double mult=vvd[i-2][vvd[i-2].size()-1]/vvd[i-2][0];
    #
    #     for( j=0;j<vvd[i-2].size()-1;j++)
    #         v.push_back(vvd[i-2][j] - vvd[i-1][j]*mult);
    #
    #     vvd.push_back(v);
    #     reverse(v.begin(),v.end());
    #     vvd.push_back(v);
    #     if(v.size()==1)
    #         break;
    # }
    #
    # for(i=0;i<vvd.size();i+=2)
    #     if(vvd[i][0]<=0)
    #         break;
    #
    # return i==vvd.size();
    vvd = []
    vvd.append(polynomial)
    vvd.append(polynomial[::-1])

    i = 2
    while True:
        v = []
        mult = vvd[i - 1][len(vvd[i - 2]) - 1] / vvd[i - 2][0]
        for j in range(len(vvd[i - 2]) - 1):
            v.append(vvd[i - 2][j] - vvd[i - 1][j] * mult)
        vvd.append(v)
        vvd.append(v[::-1])
        if len(v) == 1:
            break
        i += 2

    for i in range(0, len(vvd), 2):
        if vvd[i][0] <= 0:
            break

    return i == len(vvd)

def all_equal(x):
    return x.count(x[0]) == len(x)

def is_stable(polynomial):
    stable_roots = is_stable_roots(polynomial)
    stable_recursive = is_stable_recursive(polynomial)
    stable_jury = is_stable_jury(polynomial)
    if not all_equal([stable_roots, stable_recursive, stable_jury]):
        raise RuntimeError("results don't match for polynomial ",
                           polynomial,
                           " with results: roots - ",
                           stable_roots,
                           " recursive - ",
                           stable_recursive,
                           " jury - ",
                           stable_jury)
    return stable_roots

def random_coeffs():
    return [1] + np.random.rand(20) - 0.5

def check_surface_filters(surface_desc):
    coeffs = to_filter_coefficients(Surface(surface_desc, surface_desc), 44100)
    for c in coeffs:
        is_stable(c[1])
    return series_coeffs(coeffs)[1]

def random_coeffs_from_surface():
    return check_surface_filters(np.random.rand(8))

def main():
    for i in range(2000):
        is_stable(random_coeffs_from_surface())

def test():
    for i in [
                 [1, 1, 1, 1, 1, 1, 1, 1],
                 [0.4, 0.3, 0.5, 0.8, 0.9, 1, 1, 1],
             ]:
        is_stable(check_surface_filters(i))

if __name__ == "__main__":
    main()
