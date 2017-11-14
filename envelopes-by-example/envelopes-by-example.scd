/*
Check:

Audio MIDI setup matching sample rates
Icicle gain on Audio MIDI @ 0.5
Icicle gain on Icicle @ max
Keyboard set to CC
Keyboard bank green

*/
(
s.waitForBoot {
  s.meter;
  ~createVars.();
  ~setupMIDI.();
  s.sync;
  ~createSynthDefs.();
  s.sync;
  ~createSynths.();
  ~createGUI.();
  s.sync;
};

~setupMIDI = {
  "midisetup".postln;
  MIDIClient.init;
  MIDIIn.connectAll;
};

~createVars = {
  ~recording = false;
  ~inputActive = false;
  ~updateRecordingButton = {|isRecording|};
  ~inputButton;
  ~envelope;
  ~midiSynths = IdentityDictionary[];
  ~monitorSynths = IdentityDictionary[];
  ~recordBuf = Buffer.alloc(s, 10 * s.sampleRate);
  ~envBuf = Buffer.alloc(s, 10 * s.sampleRate);
  ~lpfcf = 0.00020355047702748;
  ~haveRecording = false;
  ~data = FloatArray.newClear(~recordBuf.numFrames);
  ~recordBus = Bus.audio(s, 1);
  ~fxBus = Bus.audio(s, 1);

  ~black = Color(0.05, 0.05, 0.05, 1.0);
  ~blue = Color(0.17, 0.61, 0.83, 1.0);
  ~grey = Color(0.3, 0.3, 0.3);
};

~createSynthDefs = {
  SynthDef(\soundIn, {|in=0, mul=1, out=0|
    Out.ar(out, SoundIn.ar(in) * mul);
  }).add;

  SynthDef(\recorder, {|in=0, buf=0, out=0|
    RecordBuf.ar(In.ar(in), buf, doneAction: 2);
  }).add;

  SynthDef(\monitor, {|in=0, out=0|
    Out.ar(out,
      In.ar(in);
    );
  }).add;

  SynthDef(\monitorInst, {|in=0, out=0 freq=440 mul=0.1 gate=1 lforate=4.85
    lfowidth=0.5 cutoff= 500 rq=0.5 pan=0.0|

    var pulse, filter, amp, freeEnv;
    pulse = Pulse.ar(freq*[1,33.5.midiratio],[0.2,0.1],[0.7,0.3]);
    amp = Amplitude.kr(In.ar(in), 0.75, 0.75);
    freeEnv = EnvGen.ar(Env([1, 1, 0], [0, 0.01], \cub, 0), gate, doneAction: 2);

    //keyboard tracking filter cutoff
    filter = BLowPass4.ar(pulse,(cutoff*(amp.squared))+200+freq,rq);
    Out.ar(out,Pan2.ar(Mix(filter)*amp*mul*freeEnv,pan));
  }).add;

  SynthDef(\choruscompresseffect, {|inout=0 gate= 1|
    var source = In.ar(inout,2);
    var env = Linen.kr(gate, 0.1, 1, 0.1, 2);
    var chorus= Splay.ar(Array.fill(4,{
      var maxdelaytime = rrand(0.005,0.02);
      DelayC.ar(
        source[0],
        maxdelaytime,
        LFNoise1.kr(Rand(0.1,0.6), 0.25 * maxdelaytime, 0.75 * maxdelaytime))
      })
    );

    chorus = Compander.ar(4*(source + chorus),source,0.4,1,4);

    XOut.ar(inout,env,chorus);

  }).add;


  ~createPlaybackSynthDef.();
};

~createSynths = {
  ~inputGroup !? { ~inputGroup.freeAll };
  ~inputGroup.free;
  ~listenGroup !? { ~listenGroup.freeAll };
  ~listenGroup.free;
  ~fxGroup !? { ~fxGroup.freeAll };
  ~fxGroup.free;

  ~inputGroup = Group(s);
  ~listenGroup = Group.after(~inputGroup);
  ~playGroup = Group(s);
  // ~fxGroup = Group.after(~listenGroup);

  ~soundInSynth = Synth.tail(~inputGroup, \soundIn, [\in, 0, \out, ~recordBus]);
  // ~chorusSynth = Synth.tail(~fxGroup, \choruscompresseffect, [\inout, ~fxBus]);
  // ~monitorSynth = Synth.tail(~listenGroup, \monitor, [\in, ~recordBus]);
};

~startRecording = {
  ~recording = true;
  ~recordingStart = SystemClock.seconds;
  ~recordSynth = Synth.tail(~listenGroup, \recorder, [
    \in, ~recordBus,
    \buf, ~recordBuf
  ]);
  ~updateRecordingButton.(~recording);
  ~envBuf.zero;
};

~stopRecording = {
  if(~recording) {
    ~recording = false;
    ~recordSynth.free;
    ~recordingDuration = SystemClock.seconds - ~recordingStart;
    ~updateRecordingButton.(~recording);
    ~haveRecording = true;
    ~setInputMode.(false);
    {
      if(~slider.visible.not) { ~slider.visible = true; };
    }.fork(AppClock);
    {
      var numFrames = (~recordingDuration * ~recordBuf.sampleRate).min(~envBuf.numFrames);
      "recording duration: %".format(~recordingDuration).postln;
      "numFrames: %".format(numFrames).postln;
      ~envBuf.zero;
      s.sync;
      ~recordBuf.copyData(~envBuf, 0, 0, numFrames);
      s.sync;
      ~recordBuf.zero;
      ~envBuf.loadToFloatArray(action: {|array|
        {
          if(array.size > 0) {
            var lastIdx = array.size - array.reverse.lastIndexForWhich {|v| v == 0.0 };
            var envData;
            ~data = array[0..lastIdx];

            envData = ~makeEnv.(~data, ~envBuf.sampleRate, ~lpfcf);
            ~envelope = envData[\env];
            ~updatePlotView.(envData);
            ~updateEditorView.();
            ~updatePlaybackSynthDefFromEditor.();
          };
        }.fork;
      });
    }.fork;
  };
};

~makeEnv = {|data, sampleRate, lpfcf|
  var abs, lpf, decimated, floor, norm, segTime, env;
  if(~haveRecording) {
    abs = data.collect {|x| x.abs; };
    lpf = FloatArray.newClear(abs.size);

    abs.do {|x, i|
      lpf[i] = if(i == 0) {
        x;
      } {
        (x * lpfcf) + (lpf[i - 1] * (1 - lpfcf))
      };
    };

    decimated = lpf.select {|x, i| i % (sampleRate / 20) == 0 };
    floor = decimated - decimated.minItem;
    norm = floor * (1 / floor.maxItem);
    if(norm[0] > 0.1) {
      norm = [0, norm].flat;
    };

    if(norm.last != 0) {
      norm = norm.add(0);
    };

    segTime = (data.size / sampleRate) / norm.size;
    env = Env(norm, segTime!norm.size, \lin);
  };

  IdentityDictionary[
    \abs -> abs,
    \norm -> norm,
    \env -> env;
  ];
};

~styleSlider = {|ezslider|
  ezslider.setColors(
    stringBackground: ~black,
    stringColor: Color.white,
    sliderBackground: ~black,
    numBackground: ~black,
    numStringColor: Color.white,
    numNormalColor: Color.white,
    numTypingColor: ~blue,
    knobColor: ~blue,
    background: ~black
  );
};

~createGUI = {
  var winExtent = 1200@723;
  var winRect = Rect((Window.screenBounds.width - winExtent.x) / 2,
                     (Window.screenBounds.height - winExtent.y) / 2,
                     winExtent.x,
                     winExtent.y);
  var remainingHeight = winExtent.y;
  var buttonExtent = ((winExtent.x - 15) / 2)@50;
  var sliderExtent = (winExtent.x - 10)@50;
  var updateRemainingHeight = {|elementHeight|
    remainingHeight = remainingHeight - elementHeight;
  };
  var plotterWidth, plotterHeight, plotterBounds;
  var editorWidth;
  var sliderRoutine, recordButton, recordButtonLabel, playButton;
  var extraLine, extraLineY;
  if (~window.notNil) {~window.close;};
  ~window = Window("", winRect).front;
  ~window.addFlowLayout;
  ~window.background = ~black;

  // inputButton
  ~inputButton = Button(~window, buttonExtent)
    .states_([
      ["Playback", Color.black, Color.white],
      ["Input", Color.white, Color.black]
    ])
    .action_({|butt|
      ~setInputMode.(butt.value === 1);
    });
  // updateRemainingHeight.(buttonExtent.y);

  // recordButton
  recordButton = UserView(~window, buttonExtent)
    .background_(Color.red(0.5));

  recordButtonLabel = StaticText(recordButton, buttonExtent)
    .string_("Record")
    .stringColor_(Color.white)
    .align_(\center);

  ~updateRecordingButton = {|isRecording|
    {
      if(isRecording) {
        recordButtonLabel.string_("Recording");
        recordButton.background_(Color.red(0.8));
      } {
        recordButtonLabel.string_("Record");
        recordButton.background_(Color.red(0.5));
      }
    }.fork(AppClock);
  };
  recordButton.mouseDownAction_({|x, y, modifiers, buttonNumber, clickCount|
    ~startRecording.();
    ~updateRecordingButton.(~recording);
  });
  recordButton.mouseUpAction_({|x, y, modifiers, buttonNumber|
    ~stopRecording.();
    ~updateRecordingButton.(~recording);
  });
  updateRemainingHeight.(buttonExtent.y);


  // slider
  ~slider = EZSlider(
    ~window,
    sliderExtent,
    "Smooth",
    ControlSpec(
      minval:1 / ~recordBuf.sampleRate,
      maxval: 1.0,
      warp: \exp,
      step: 0.0,
      default: ~lpfcf
    ),
    numberWidth: 70
  );

  ~styleSlider.(~slider);
  ~slider.visible = false;


  ~updatePlotView = {|envData|
    {
      ~plotterSig.value = envData[\abs];
      ~plotterLpf.value = envData[\norm];
      [~plotterSig, ~plotterLpf].do {|plotter|
        plotter.plots.do { |plot, i|
          plot.gridOnX = false;
          plot.gridOnY = false;
          plot.backgroundColor = ~black;
          plot.plotColor = ~blue
        };
        plotter.refresh;
      };
    }.fork(AppClock);
  };

  ~updateEditorView = {
    {
      ~editorViewWrap.visible = false;
      if(~envelope.notNil) {
        var data = ~envelope.levels;
        var start = data.detectIndex {|lvl| lvl > 0.01};
        var end = data.size - data.reverse.detectIndex {|lvl| lvl > 0.01 };

        data = data[start..end];
        ~editorView.data_(data);
        ~editorView.intervalSlider.valueAction_(~editorView.currentInterval);
      };
      ~editorViewWrap.visible = true;
    }.fork(AppClock);
  };

  ~updatePlaybackSynthDefFromEditor = {
    if(~envelope.notNil) {
      var env = ~editorView.currentEnv(~envelope.times.first);
      ~createPlaybackSynthDef.(env);
    };
  };

  ~slider.numberView.decimals = 6;
  ~slider.action_({|ez|
    ~lpfcf = ez.value;
    sliderRoutine.stop;
    sliderRoutine = {
      var envData = ~makeEnv.(~data, ~recordBuf.sampleRate, ~lpfcf);
      ~envelope = envData[\env];
      // 0.2.wait;
      {
        ~playGroup.set(\gate, 0);
        0.5.wait;
        ~playGroup.freeAll;
      }.fork;

      ~updatePlotView.(envData);
      ~updateEditorView.();
      ~updatePlaybackSynthDefFromEditor.();
    }.fork;
  });
  updateRemainingHeight.(sliderExtent.y);

  // plotter
  plotterWidth = 1 + (~window.bounds.width / 2) - (~window.view.decorator.margin.x * 2);
  plotterHeight = 200;
  plotterBounds = Rect(1, 1, plotterWidth - 2, plotterHeight - 2);

  ~plotterSigWrap = UserView(~window, plotterWidth@plotterHeight);
  ~plotterSigWrap.background = ~grey;
  ~plotterSig = Plotter("", plotterBounds, ~plotterSigWrap);
  ~plotterSig.interactionView.background = ~black;
  ~plotterSig.superpose = false;
  ~plotterSig.interactionView.resize = 0;

  ~plotterLpfWrap = UserView(~window, plotterWidth@plotterHeight);
  ~plotterLpfWrap.background = ~grey;
  ~plotterLpf = Plotter("", plotterBounds, ~plotterLpfWrap);
  ~plotterLpf.interactionView.background = ~black;
  ~plotterLpf.superpose = false;
  ~plotterLpf.interactionView.resize = 0;

  updateRemainingHeight.(plotterHeight);

  // editor view
  // w.bounds.width@remainingHeight;

  editorWidth = (~window.bounds.width - (~window.view.decorator.margin.x * 2) - 2);
  ~editorViewWrap = UserView(~window, editorWidth@400);
  ~editorViewWrap.background = ~grey;
  ~editorView = MUISmootherView(~editorViewWrap, Rect(1,
                                                      1,
                                                      editorWidth - 2,
                                                      ~editorViewWrap.bounds.height - 2))
    .updateAction_({
      ~updatePlaybackSynthDefFromEditor.();
    });
  ~editorView.view.background = ~black;
  ~editorView.backgroundView.background = ~black;

  extraLineY = ~editorView.envRect.bounds.bottom + 1;
  extraLine = UserView(~editorView.view, Rect(0, extraLineY, ~editorView.view.bounds.width, 1));
  extraLine.background = ~grey;

  ~styleSlider.(~editorView.intervalSlider);
};

~createPlaybackSynthDef = {arg envelope=Env.asr;
  if(envelope.notNil) {
    SynthDef(\prophet5pwmstrings,{|out=0 freq=440 mul=1.0 gate=1 lforate=10
      lfowidth=0.5 cutoff=12000 rq=0.5 pan=0.0|

      var lfo, pulse, filter, env;

      lfo = LFTri.kr(lforate * [1,1.01], Rand(0, 2.0) ! 2);
      pulse = Pulse.ar(freq * [1,1.01], lfo * lfowidth + 0.5);
      filter = RLPF.ar(pulse, cutoff, rq);
      env = EnvGen.ar(envelope, gate, doneAction:2);

      Out.ar(out,
        Pan2.ar(Mix(filter) * env * mul * 0.5, pan)
      );
    }).add;

    SynthDef(\everythingrhodes, {|out=0 freq=440 mul=0.1 gate=1 lforate=4.85
      lfowidth=0.5 cutoff= 500 rq=0.5 pan=0.0|

      var pulse, filter, env;
      pulse = Pulse.ar(freq*[1,33.5.midiratio],[0.2,0.1],[0.7,0.3]);
      env = EnvGen.ar(envelope, gate, doneAction:2);

      //keyboard tracking filter cutoff
      filter = BLowPass4.ar(pulse,(cutoff*(env.squared))+200+freq,rq);
      Out.ar(out,Pan2.ar(Mix(filter)*env*mul,pan));
    }).add;
  };
};

MIDIdef.noteOn(\noteOnAction, {|vel, num|
  var freq = num.midicps;

  if(~inputActive || ~envelope.isNil) {
    if(~inputActive && ~recording.not) {
      ~startRecording.();
    };

    ~monitorSynths[num].set(\gate, 0);
    ~monitorSynths[num] = Synth.tail(~listenGroup, \monitorInst, [
      \in, ~recordBus,
      \freq, freq,
      \mul, 1,
      \rq, vel.linlin(0, 127, 0.8, 0.2),
      \lforate, 0.2,
      \lfowidth, 0.5
    ]);
  } {
    // playback mode
    ~midiSynths[num].set(\gate, 0);
    ~midiSynths[num] = Synth.tail(~playGroup, \everythingrhodes, [
    // ~midiSynths[num] = Synth(\prophet5pwmstrings, [
      \freq, freq,
      \mul, vel.lincurve(0, 127, 0, 0.3, 2),
      \rq, 0.5,
      \lforate, 0.2,
      \lfowidth, 0.5
    ]);
  }
});

MIDIdef.noteOff(\noteOffAction, {|vel, num|
  ~midiSynths[num].set(\gate, 0);
  ~midiSynths.removeAt(num);
  ~monitorSynths[num].set(\gate, 0);
  ~monitorSynths.removeAt(num);

  if(~inputActive && ~recording) {
    ~stopRecording.();
  };
});

~setInputMode = {|active|
  ~inputActive = active;
  if(~inputActive) {"input!"} {"playback!"}.postln;
  {~inputButton.value_(if(active) {1} {0});}.fork(AppClock);
};

~toggleInput = {
  ~setInputMode.(~inputActive.not);
};


MIDIdef.cc(\cc, {|val, num, chan, src|
  var sliderVal = ~slider.controlSpec.map(val.linlin(0, 127, 0, 1));
  {~slider.value_(sliderVal)}.fork(AppClock);
  ~lpfTimeout.stop;
  ~lpfTimeout = {
    ~slider.valueAction_(sliderVal);
  }.fork(AppClock);
}, 1);

MIDIdef.cc(\inputToggle, {|val, num, chan, src|
  var state = val.clip(0, 1);
  {~inputButton.valueAction_(state)}.fork(AppClock);
}, 27);

MIDIdef.cc(\interval, {|val, num, chan, src|
  ~intervalSliderMIDIRout.stop;
  if(~envelope.notNil) {
    ~intervalSliderMIDIRout = {
      var interval;
      0.3.wait;
      interval = ~editorView.intervalSpec.map(val.linlin(0, 127, 0, 1));
      ~editorView.intervalSlider.valueAction_(interval);
    }.fork(AppClock)
  };
}, 5);


MIDIdef.cc(\all, {|val, num, chan, src|
  num.postln;
}, nil);

);


/*
TODO:
- fix find handles with small number of points
- fix view switching
- ui on resize



FUTURE:
- real time performance on lpf cf
- better lpf, attack and release controls. lagUD style
- decimate based on derivative
*/
