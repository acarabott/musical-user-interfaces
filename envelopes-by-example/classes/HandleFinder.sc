// ported from original Squeak version by Yoshiki Ohshima
HandleFinder {
  *errorBetween {|startIndex, endIndex, levels|
    var n = endIndex - startIndex;
    var aa = 0;
    var ax = 0;
    var ay = 0;
    var ab = 0;
    var ac = 0;
    var error = 0;
    var a, b;
    var yk;

    (startIndex..endIndex-1).do {|xk|
      yk = levels[xk];
      aa = aa + (xk * yk);
      ax = ax + xk;
      ay = ay + yk;
      ab = ab + (xk * xk);
      ac = ac + xk;
    };

    a = ((n.asFloat * aa) - (ax * ay)) / ((n * ab) - (ac * ac));
    b = (ab.asFloat * ay) - (aa * ax) / ((n * ab) - (ac * ac));

    (startIndex..endIndex-1).do {|xk|
      var diff = (a * xk + b) - levels[xk];
      error = error + (diff * diff);
    };

    ^error;
  }

  *isPeakAt {|index, levels, window|
    var leftScore = 0;
    var rightScore = 0;
    var peak = false;
    var half = (window / 2).asInt + 1;

    (window - 1, window - 2..0).do {|i|
      (levels[index - i] < levels[index]).if { leftScore = leftScore + 1 };
      (levels[index] > levels[index + i]).if { rightScore = rightScore + 1 };
    };

    (leftScore >= half && (rightScore >= half)).if {
      peak = true;
      (index - half..index + half).do {|i|
        (levels[index] < levels[i]).if {
          peak = false;
        }
      };
    };

    ^peak;
  }

  *isValleyAt {|index, levels, window|
    var leftScore = 0;
    var rightScore = 0;
    var valley = false;
    var half = (window / 2).asInt + 1;

    (window - 1, window - 2..0).do {|i|
      (levels[index - i] > levels[index]).if { leftScore = leftScore + 1 };
      (levels[index] < levels[index + i]).if { rightScore = rightScore + 1 };
    };

    (leftScore >= half && (rightScore >= half)).if {
      valley = true;
      (index - half..index + half).do {|i|
        (levels[index] > levels[i]).if { valley = false; }
      };
    };

    ^valley;
  }

  // given an array of levels, returns an array of indices of 'interesting' points
  *findHandles {|levels, interval=3|
    var residuals, maybeThere, start, e, error, index, min, resultIdxs, result,
    s, peak, peaks, bigErrors, close, peaksAndValleys, valley, valleys, mid;

    var tooFewLevels = levels.size <= (interval * 2);
    var tooSmallInterval = interval < 3;
    (tooFewLevels || tooSmallInterval).if {
      ^(0..levels.size - 1);
    };


    residuals = (0..(levels.size - interval)).collect {|i|
      this.errorBetween(i, i + interval, levels);
    };

    start = interval + 1;
    bigErrors = List[];

    while({start < (residuals.size - interval)}, {
      peak = this.isPeakAt(start, residuals, interval);
      valley = this.isValleyAt(start, residuals, interval);
      (peak || valley).if {
        bigErrors.add(start);
        start = start + interval;
      } {
        start = start + 1;
      };
    });

    peaks = List[];

    (interval .. (levels.size - interval)).do {|p|
      this.isPeakAt(p, levels, interval).if {
        peaks.add(p);
      };
    };

    valleys = List[];
    (interval .. (levels.size - interval)).do {|p|
      this.isValleyAt(p, levels, interval).if {
        valleys.add(p);
      };
    };

    peaksAndValleys = SortedList[];
    peaksAndValleys.addAll(peaks);
    peaksAndValleys.addAll(valleys);

    maybeThere = SortedList.newFrom(peaksAndValleys);
    maybeThere.add(0);
    maybeThere.add(levels.size - 1);

    bigErrors.do {|m|
      peaksAndValleys.detect {|p|
        (m - p).abs < ((interval / 2).asInt + 1);
      }.isNil.if {
        maybeThere.add(m);
      };
    };

    resultIdxs = List[];

    // we need to be able to look at three points: start, middle, end
    if(maybeThere.size < 3) {
      resultIdxs.addAll(maybeThere);
    } {
      (0..(maybeThere.size - 3)).do {|i|
        s = maybeThere[i];
        e = maybeThere[i + 2];
        mid = maybeThere[i + 1];

        peaksAndValleys.includes(mid).if {
          // if the mid point is a peak or valley, just use it
          resultIdxs.add(mid);
        } {
          // jiggle to find best mid point
          min = inf;
          index = 0;
          // look *within* range, avoiding potential control point at s
          (s + 1 .. (e - 1)).do {|n|
            error = this.errorBetween(s, n, levels) + this.errorBetween(n, e, levels);
            (error < min).if {
              min = error;
              index = n;
            };
          };
          resultIdxs.add(index);
        };
      };
      resultIdxs.add(levels.size - 1);
    };

    if(levels[resultIdxs.first] > 0.01) {
      resultIdxs = [0, resultIdxs].flat;
    };

    ^resultIdxs;
  }
}
