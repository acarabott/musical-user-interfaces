~main.();

(
// Run this code first
// -----------------------------------------------------------------------------
~main = {
  ~setupVars.();
  s.waitForBoot {
    {
      s.meter;
      ~loadBuffers.();
      ~addSynthDefs.();
      s.sync;
      ~createSynths.();
      s.sync;
      StageLimiter.activate;
      ~createGUI.();
      ~addMidi.();
    }.fork(AppClock);
  };
};

s.options.memSize = 2**20;
MIDIClient.init;
MIDIIn.connectAll;

~setupVars = {
  ~minMul = 0.1;
  ~maxMul = 0.9;
  ~playWithTap = false;
  ~comb = false;
  ~normalize = false;
  ~onsetsEnabled = false;
  ~onsetsTempDisabled = false;
  ~taps = List[];
  ~addTap = {|time, mul|
    if(~taps.size < 50) {
      ~taps.add([time, mul])
    };
  };
  ~delayViewDuration = 5;
  ~recording = false;
  ~recordingStartTime = 0.0;
  ~recordingCurrentTime = 0.0;
  ~delaySynths = [];
  ~inputSynths = Array.newClear(127);
  ~inputStartTime = 0.0;
  ~settings = [
    IdentityDictionary[
      \threshold -> 0.05,
      \odftype -> \rcomplex,
      \relaxtime -> 0.1,
      \floor -> 0.5,
      \mingap ->  10,
      \medianspan -> 10,
      \whtype -> 1
    ],
    IdentityDictionary[
      \threshold -> 0.1,
      \odftype -> \wphase,
      \relaxtime -> 0.1,
      \floor -> 0.15,
      \mingap ->  50,
      \medianspan ->  10,
      \whtype ->  1
    ],

    IdentityDictionary[
      \threshold -> 0.05,
      \odftype -> \mkl,
      \relaxtime -> 0.1,
      \floor -> 0.1,
      \mingap ->  50,
      \medianspan ->  11,
      \whtype ->  1
    ],
    IdentityDictionary[
      \threshold -> 0.1,
      \odftype -> \rcomplex,
      \relaxtime -> 0.1,
      \floor -> 0.2,
      \mingap ->  5,
      \medianspan -> 10,
      \whtype -> 1
    ]
  ];
};

~loadBuffers = {
  var path = "/Users/ac/Projects/harc/projects/mui/mui/audio";
  ~guitarNotes = Buffer.read(s, path +/+ "guitar-for-delay-norm.wav");
  ~guitarDelayStrum = Buffer.read(s, path +/+ "guitar-delay-strum-norm.wav");
  ~guitarKanjira = Buffer.read(s, path +/+ "guitar-kanjira-norm.wav");
};

~addSynthDefs = {
  SynthDef(\loopbuf, {|buf=0, out=0|
    var sig = LoopBuf.ar(1, buf, BufRateScale.kr(buf), 1, 0, 0, BufFrames.kr(buf));
    Out.ar(out,
      sig
    );
  }).add;

  SynthDef(\soundIn, {|in=0, mul=1.0,out=0|
    Out.ar(out, SoundIn.ar(in) * mul);
  }).add;

  SynthDef(\singleDelay, {|in=0, delaytime=0.1, mul=1.0, out=0|
    Out.ar(out, DelayN.ar(In.ar(in), 15, delaytime) * mul);
  }).add;

  SynthDef(\singleComb, {|in=0, delaytime=0.1, decay=2, mul=0.1, out=0|
    Out.ar(out, CombN.ar(In.ar(in), 15, delaytime, decay, mul));
  }).add;

  SynthDef(\singleShot, {|buf=0, mul=1.0, out=0|
    Out.ar(out,
      Mix(PlayBuf.ar(2, buf, BufRateScale.kr(buf), doneAction:2))
    );
  }).add;

  SynthDef(\keys, {|in=0, freq=440, gate=1, mul=1.0, out=0|
    var freqs = [freq / 2, freq, freq * 1.015, freq * 0.99];
    var muls = [0.8, 0.5, 0.2, 0.2];
    var tris = LFTri.ar(freqs, 0, muls);
    var sig = Mix((tris * 1).tanh(0.8));
    var env = Env.asr(0.05, 1.0, 0.8);
    var envSig = EnvGen.ar(env, gate, doneAction: 2);
    var ffreq = EnvGen.kr(Env([2100, 200], [env.duration * 0.5], 'exp'));
    var rq = EnvGen.kr(Env([0.2, 0.6], [env.duration * 0.5], 'exp'));
    var filt = RLPF.ar(sig, ffreq, rq);

    Out.ar(out,
      (filt * envSig * mul);
    );
  }).add;

  SynthDef(\everythingrhodes, {|out=0 freq=440 mul=0.1 gate=1 lforate=4.85
    lfowidth=0.5 cutoff= 500 rq=0.5 pan=0.0|

    var pulse, filter, env;
    pulse = Pulse.ar(freq*[1,33.5.midiratio],[0.2,0.1],[0.7,0.3]);
    env = EnvGen.ar(Env.asr, gate, doneAction:2);

    //keyboard tracking filter cutoff
    filter = BLowPass4.ar(pulse,(cutoff*(env.squared))+200+freq,rq);
    Out.ar(out,Pan2.ar(Mix(filter)*env*mul,pan));
  }).add;

  SynthDef(\loop, {|buf=0, mul=1.0, out=0|
    Out.ar(out,
      Mix(LoopBuf.ar(2, buf, BufRateScale.kr(buf), 1, 0, 0, BufFrames.kr(buf)))
    );
  }).add;

  SynthDef(\dry, {|in=0, mul=1.0, out=0|
    Out.ar(out, In.ar(in) );
  }).add;

  SynthDef(\stereo, {|in=0, out=0|
    Out.ar(out,
      In.ar(in).dup;
    );
  }).add;

  SynthDef(\onsets, {|in=0, ampAtt=0.0, ampRel=0.5 threshold=0.5,
    odftype=\rcomplex, relaxtime=1, floor=0.1, mingap=10, medianspan=11,
    whtype=1, rawodf=0, out=0|

    var sig = In.ar(in);
    var amp = Amplitude.kr(sig, 0, 0.5);
    var fft = FFT(LocalBuf(512), sig);
    var onsets = Onsets.kr(fft, threshold, \wphase, relaxtime, floor, mingap,
      medianspan, whtype, rawodf);

    SendReply.kr(onsets, '/onset', [amp]);
  }).add;
};

~createSynths = {
  ~inputBus = Bus.audio(s, 1);
  ~mixBus = Bus.audio(s, 1);
  ~inputGroup = Group(s);
  ~delayGroup = Group.after(~inputGroup, s);
  ~soundIn = Synth.tail(~inputGroup, \soundIn, [\in, 0, \out, ~inputBus]);
  ~dry = Synth.after(~inputGroup, \dry, [\in, ~inputBus, \out, ~mixBus]);
  ~stereo = Synth.after(~delayGroup, \stereo, [\in, ~mixBus]);
  ~onsetSynth = Synth.after(~inputGroup, \onsets, [
    \in, ~inputBus,
    \threshold, 0.15,
    \odftype, \rcomplex,
    \relaxtime, 0.1,
    \floor, 0.5,
    \mingap,  12,
    \medianspan, 10,
    \whtype, 1
  ]);
};


~createGUI = {
  if(~win.notNil) { ~win.close; };
  ~rect = Rect(0, 0, 1500, 750);
  ~win = Window("", ~rect).front;

  ~win.view.onResize_({|view|
    ~delayView.bounds = view.bounds.extent;
  });

  ~win.view.keyDownAction_({|view, char, modifiers, unicode, keycode, key|
    if (char == ($ )) {
      if (~recording) {
        ~stopRecording.();
      } {
        ~startRecording.();
      }
    };
  });

  // ~win.view.keyDownAction_({|view, char, mod, uni, key|
  //   ~inputAction.(rrand(0.25, 1.0));
  // });

  ~delayView = UserView(~win, ~win.bounds.extent)
    .background_(Color.gray(0.2))
    .drawFunc_({|view|
      var viewDuration = ~delayViewDuration * 1.25;
      ~taps.do {|tap, i|
        var time = tap[0];
        var mul = tap[1];
        var x = (time / viewDuration) * view.bounds.width;
        var y = (1 - mul) * view.bounds.height;

        Pen.width = mul.linlin(0, 1, 1.5, 6, 2);
        Pen.strokeColor = Color.gray(mul.linlin(0, 1, 0.4, 1));
        Pen.moveTo(x@y);
        Pen.lineTo(x@view.bounds.height);
        Pen.stroke;
      };

      {
        var x = (~recordingCurrentTime / viewDuration) * view.bounds.width;
        Pen.width = 2;
        Pen.strokeColor = Color.red;
        Pen.moveTo(x@0);
        Pen.lineTo(x@view.bounds.height);
        Pen.stroke;
      }.();
    });

  Tdef(\update, {
    inf.do {|i|
      if(~recording) {
        var tapTimes = ~taps.collect {|tap| tap[0]};
        ~recordingCurrentTime = SystemClock.seconds - ~recordingStartTime;
        ~delayViewDuration = max(~recordingCurrentTime, tapTimes.maxItem ?? 5);
      };
      {~delayView.refresh}.fork(AppClock);
      0.016.wait;
    }
  }).play(SystemClock);
};

~startRecording = {
  if(~recording.not) {
    ~recording = true;
    ~recordingStartTime = SystemClock.seconds;
    // if(~taps.isEmpty) {
    //   ~loop = Synth.before(~delayGroup, \loop, [\buf, ~buf, \out, ~loopBus]);
    // };
    ~recordTap = {|mul=1.0|
      var time = SystemClock.seconds - ~recordingStartTime;
      ~addTap.(time, mul);
    };
  };
};

~stopRecording = {
  if(~recording) {
    ~recording = false;
    ~recordTap = {};
    ~recordingCurrentTime = 0.0;
    if(~normalize) {~normalizeDelayMuls.();};
    ~createDelays.();
  };
};

~createDelays = {
  ~delaySynths = ~taps.collect {|tap|
    var time = tap[0];
    var mul = tap[1];
    var key = if(~comb) { \singleComb } { \singleDelay };

    Synth.tail(~delayGroup, key, [
      \in, ~inputBus,
      \delaytime, time,
      \mul, mul,
      \out, ~mixBus
    ]);
  };
};

~clearDelays = {
  ~taps = List[];
  ~clearDelaySynths.();
};

~clearDelaySynths = {
  ~delayGroup.freeAll;
  ~delaySynths = [];
};

~addMidi = {
  MIDIdef.program(\program, {|val, num, chan, src|

  });

  MIDIdef.noteOn(\noteOn, {|val, num, chan, src|
    ~inputSynths[num].set(\gate, 0);
    if(~onsetsEnabled.not) {
      ~inputSynths[num] = Synth.tail(~inputGroup, \everythingrhodes, [
        \freq, num.midicps,
        \mul, val.lincurve(0, 127, 0, 0.1, 2),
        \out, ~inputBus,
        \cutoff, rrand(50, 700),
        \rq, rrand(0.3, 0.5)
      ]);
    };
    val.postln;
    ~inputAction.(val.lincurve(0, 127, 0, 1, 3), num);
  });

  MIDIdef.noteOff(\noteOff, {|val, num, chan, src|
    ~inputSynths[num].set(\gate, 0);
  });
};

~inputAction = {|mul, midiNote=60|
  var inStartWindow;
  var canRecord = ~recording.not && ~taps.isEmpty;
  var synth;

  if(canRecord) {
    ~inputStartTime = SystemClock.seconds;
    ~startRecording.();
  };

  inStartWindow = (SystemClock.seconds - ~inputStartTime) < 0.01;

  // if(~playWithTap && (inStartWindow || canRecord.not)) {
  if(~playWithTap && ~recording) {
    synth = Synth.tail(~inputGroup, \everythingrhodes, [
      \freq, midiNote.midicps,
      \mul, mul,
      \out, ~inputBus
    ]);
    ~inputSynths[midiNote] = synth;
    {
      0.5.wait;
      synth.set(\gate, 0);
    }.fork;
  };

  if(~recording && inStartWindow.not) {
    ~recordTap.(mul);
  };

  synth;
};

~normalizeDelayMuls = {
  var maxMul = ~taps.collect {|tap| tap[1]; }.maxItem;
  ~taps = ~taps.collect {|tap| tap[1] = tap[1].linlin(0, maxMul, ~minMul, ~maxMul)};
  ~clearDelaySynths.();
  ~createDelays.();
};

~onsetBeep = false;
OSCdef(\onset, {|msg, time, addr, recvPort|
  var mul = msg[3].lincurve(0, 1, 0, 1, 0);
  if(~onsetsEnabled && ~onsetsTempDisabled.not) {
    ~inputAction.(mul);
  };
  if(~onsetBeep) {
    Synth(\beep, [\freq, 440, \mul, 0.2, \pan, 0, \att, 0.001, \rel, 0.2, \out, 0]);
  };
}, '/onset');

MIDIdef.cc(\pedal, {|val, num, chan, src|
  var down = val == 127;
  var up = val == 0;

  if(down) {
    "pedal down!".postln;
    ~stopRecording.();
    ~onsetBeep = true;
    ~onsetsTempDisabled = true;
  };
  if(up) {
    "pedal up!".postln;
    ~onsetBeep = false;
    ~onsetsTempDisabled = false;
  };
}, 64);

MIDIdef.program(\clear, {|val, num, chan, src|
  val.postln;
  if(val == 3) {
    "clear delays".postln;
    ~clearDelays.();
  };
  if(val == 2) {
    ~onsetsEnabled = ~onsetsEnabled.not;
    ~normalize = ~onsetsEnabled;
    if(~onsetsEnabled) {
      "Onsets".postln;
    } {
      "Keyboard".postln;
    };
  };
  if(val == 0) {
    ~stopRecording.();
  };
  if(val == 1) {
    ~comb = ~comb.not;
    ~clearDelaySynths.();
    ~createDelays.();
    if(~comb) {
      "Comb".postln;
    } {
      "Single".postln;
    };
  }
});


);

MIDIdef.cc(\all, {|val, num, chan, src|
  num.postln;
});

// variables for tweaking settings
// -----------------------------------------------------------------------------
~stopRecording.()
~clearDelays.();

~playWithTap = false;
~playWithTap = true;
~comb = false;
~comb = true;
~normalize = false;
~normalize = true;
~maxMul = 0.9;
~minMul = 0.1;
~delayGroup.set(\decay, 2, \mul, 0.1);

// enable onsets
(
~onsetsEnabled = true;
~normalize = true;
~playWithTap = false;
);

// disable onsets
(
~onsetsEnabled = false;
~normalize = false;
~playWithTap = true;
)

/*
Notes

29-08-16

Onsets don't feel reliable enough, need to tweak to find right settings,
particularly getting the amplitude right is critical.

Should try having each delay play back the signal that was used to start it,
will need to have some kind of window on this duration.


Need to have after the fact editing
  - deleting / adjusting taps
  - normalizing / scaling amplitudes

Feedback would be good to have
  - difference is that we don't have a sense of end point
  - have to think about this...
  - maybe just use first time interval (start to first tap)
    - this would be a good way to get feedback with only two hits
      - (cheap tap tempo)
    - this from the last node?

*/
