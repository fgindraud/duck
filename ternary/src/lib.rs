use bitvec::view::BitView;

#[derive(Debug, Clone, Copy)]
pub struct Ternary64 {
    /// i-th bit : is trit i a +1 ?
    positive: u64,
    /// i-th bit : is trit i a -1 ?
    negative: u64,
}

impl std::fmt::Binary for Ternary64 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let text_repr: String = std::iter::zip(
            self.positive.view_bits::<bitvec::order::Msb0>(),
            self.negative.view_bits::<bitvec::order::Msb0>(),
        )
        .map(|(p, n)| match (*p, *n) {
            (true, false) => '+',
            (false, true) => '-',
            _ => '0',
        })
        .skip_while(|c| *c == '0')
        .collect();
        f.pad_integral(true, "0t", &text_repr)
    }
}

impl From<Ternary64> for i128 {
    fn from(mut ternary: Ternary64) -> i128 {
        let mut i = 0;
        let mut power_of_3: i128 = 1;
        while std::ops::BitOr::bitor(ternary.positive, ternary.negative) != 0 {
            if ternary.positive & 1 == 1 {
                i += power_of_3
            }
            if ternary.negative & 1 == 1 {
                i -= power_of_3
            }
            ternary.positive >>= 1;
            ternary.negative >>= 1;
            power_of_3 *= 3;
        }
        i
    }
}

impl From<u64> for Ternary64 {
    fn from(binary: u64) -> Ternary64 {
        // bit i = 1 ->
        todo!()
    }
}

impl From<i64> for Ternary64 {
    fn from(binary: i64) -> Ternary64 {
        todo!()
    }
}

impl Ternary64 {
    fn normalize(&mut self) {
        // Cancel bits that are +1 and -1 at same position
        let colliding_bits = std::ops::BitAnd::bitand(self.positive, self.negative);
        let kept_bits = std::ops::Not::not(colliding_bits);
        self.positive &= kept_bits;
        self.negative &= kept_bits
    }
}

impl std::ops::Neg for Ternary64 {
    type Output = Ternary64;
    fn neg(self) -> Self::Output {
        Ternary64 {
            positive: self.negative,
            negative: self.positive,
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::Ternary64;

    #[test]
    fn debug() {
        let ternary = Ternary64 {
            positive: 0b10,
            negative: 0b01,
        };
        println!("{:06b}", ternary);
        assert_eq!(i128::from(ternary), 2);
    }
}
