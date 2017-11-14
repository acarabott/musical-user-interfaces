// TODO this should it's own grouping UserView, but meh
MUISmootherView {
  var <>parent;
  var <>view;
  var <>backgroundView;
  var <>envRect;
  var <>controlsView;
  var <>controlsRect;
  var <>intervalSpec;
  var <>intervals;
  var <>currentInterval;
  var <>envViews;
  var <>showMultiple;
  var <>intervalSlider;
  var <>showMultiWidget;
  var <data;
  var <updateAction;
  var <enabledColor;

  *new {|parent, bounds|
    ^super.new.init(parent, bounds);
  }

  init {|aParent, aBounds|
    var controlsHeight = 60;
    var marginY = 5;
    parent = aParent;
    view = UserView(parent, aBounds);
    showMultiple = false;
    envRect = Rect(0, 0, view.bounds.width, view.bounds.height - controlsHeight);
    backgroundView = UserView(view, envRect).background_(Color.clear);
    controlsRect = Rect(0, envRect.bounds.bottom + 2, view.bounds.width, controlsHeight);
    controlsView = UserView(view, controlsRect);
    intervalSpec = ControlSpec(minval:10, maxval: 2, warp: \lin, step: 1, default: 3);
    intervals = (intervalSpec.maxval..intervalSpec.minval);
    currentInterval = intervalSpec.default;
    enabledColor = Color.white;

    showMultiWidget = Button(controlsView, Rect(4, marginY, 100, controlsHeight - (marginY * 2)))
      .states_([
        ["Show variations", Color.white, Color(0.17, 0.61, 0.83, 1.0)],
        ["Hide variations", Color.black, Color.white]
      ])
      .action_({|button|
        showMultiple = button.value === 1;
        intervalSlider.visible = showMultiple;
        this.toggleViews;
      })
      .visible_(false);

    intervalSlider = EZSlider(controlsView,
        Rect(showMultiWidget.bounds.right + 10,
             marginY,
             controlsRect.width - showMultiWidget.bounds.width - 18,
             controlsHeight - (marginY * 2)),
        "interval",
        intervalSpec,
        layout:\horz
      ).action_({|ez|
        currentInterval = ez.value.asInt;
        this.toggleViews;
        updateAction.();
      })
      .visible_(showMultiple);
    intervalSlider.labelView.align_(\left);
  }

  data_ {|aData|
    data = aData;
    data[0] = 0;
    this.removeViews;
    this.createViews;
    showMultiWidget.visible = true;
    ^this;
  }

  removeViews {
    envViews.do {|ev| ev.remove; };
    envViews = [];
  }

  createViews {
    envViews = intervals.collect {|interval, i|
      var enabled = interval == currentInterval;
      MUIEnvView(view, envRect, enabled)
        .data_(data, false)
        .interval_(interval)
        .color_(Color.hsv(i.linlin(0, intervals.size, 0.0, 1.0), 0.5, 0.8))
        .visible_(showMultiple || enabled)
        .updateAction_(updateAction)
        .enabledColor_(enabledColor);
    };
  }

  recreateViews {
    var allStates, bgStates, curState, orderedStates;

    envViews.do {|e| e.view.remove; };
    allStates = envViews.collect {|e| e.state; };
    // ensure current one is last to receive mouse actions
    bgStates = allStates.select {|state| state['interval'] != currentInterval};
    curState = allStates.select {|state| state['interval'] == currentInterval};
    orderedStates = [bgStates, curState].flat;
    envViews = orderedStates.collect {|state|
      MUIEnvView(view, envRect, false)
        .state_(state)
        .updateAction_(updateAction)
        .enabledColor_(enabledColor);
    };
  }

  calcAlphaFromInterval {|interval|
    var dist = (interval - currentInterval).abs;
    ^dist.lincurve(0, intervals.size, 0.6, 0.05, -5);
  }

  updateCurrentView {
    envViews.do {|envView, i|
      var on = envView.interval == currentInterval;
      envView
        .enabled_(on)
        .visible_(showMultiple || on)
        .alpha_(this.calcAlphaFromInterval(envView.interval));
    };
  }

  toggleViews {
    this.recreateViews;
    this.updateCurrentView;
  }

  currentView {
    var current = envViews.select {|ev| ev.interval == currentInterval; }.first;
    current = current ?? { envViews.first; };
    ^current;
  }

  currentEnv {|segTime=0.05, curves=\lin|
    if(this.currentView.notNil) {
      ^this.currentView.env(segTime, curves);
    };
    ^nil;
  }

  setViewUpdateActions {
    envViews.do {|ev| ev.updateAction_(updateAction)};
  }

  updateAction_ {|aUpdateAction|
    updateAction = aUpdateAction;
    this.setViewUpdateActions;
    ^this;
  }

  enabledColor_ {|aEnabledColor|
    enabledColor = aEnabledColor;
    envViews.do {|envView|
      envView.enabledColor = enabledColor;
    };
  }
}
