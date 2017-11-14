MUIEnvView {
  var <>parent;
  var <>bounds;
  var <>view;
  var <data;
  var <interval;
  var <>handles;
  var <>handleIdxs;
  var <>lineWidth;
  var <color;
  var <enabledColor;
  var <alpha;
  var <enabled;
  var <visible;
  var <updateAction;
  var <envLevels;

  var <>neighbourDistance = 1000;
  var <>ccurve = -2;

  *new {|parent, bounds, enabled=true|
    ^super.new.init(parent, bounds, enabled);
  }

  init {|aParent, aBounds, aEnabled|
    parent = aParent;
    bounds = aBounds;
    view = UserView(parent, bounds).background_(Color.clear);
    data = [];
    envLevels = [];
    lineWidth = 1;
    interval = 3;
    color = Color.gray;
    enabledColor = Color.black;
    alpha = 1.0;
    this.setDrawFunc;
    this.enabled = aEnabled;
    this.visible = true;
  }

  setDrawFunc {
    view.drawFunc = {|view|
      Pen.width = lineWidth;
      Pen.strokeColor = enabled.if { enabledColor; } { color.alpha_(alpha) };
      Pen.moveTo(0@view.bounds.height);
      handles.do {|handle| Pen.lineTo(handle.center)};
      Pen.stroke();
    };
  }

  data_ {|aData, recreate=true|
    data = aData.shallowCopy;
    if(recreate) {
      this.recreateHandles;
    };
    ^this;
  }

  remove {
    view.remove;
  }

  removeHandles {
    handles.do {|h| h.remove; }
  }

  setHandleActions {
    handles.do {|handle, i|
      handle.action_({|normPoint|
        envLevels[i] = normPoint.y;
        view.refresh;
        updateAction.();
      });
    };
  }

  recreateHandles {
    this.removeHandles;
    this.createHandleIdxs;
    this.createEnvLevels;
    this.createHandles;
  }

  createHandleIdxs {
    handleIdxs = HandleFinder.findHandles(data, interval);
  }

  createEnvLevels {
    envLevels = data.atAll(handleIdxs);
  }

  envTimes {|step=0.05|
    ^handleIdxs[1..].collect {|idx, i|
      (idx - handleIdxs[i]) * step;
    };
  }

  envLevels_ {|aEnvLevels, recreate=true|
    envLevels = aEnvLevels;
    if(recreate) {
      this.recreateHandles;
    };
  }

  getPointFromLevel {|level, i, size|
    var x = (i / size) * view.bounds.width;
    var y = view.bounds.height - (level * view.bounds.height);
    ^x@y;
  }

  createHandles {
    handles = envLevels.collect {|level, i|
      var point = this.getPointFromLevel(level, i, envLevels.size);
      MUIHandle(view, point, enabled);
    };
    this.setHandleActions;
    view.refresh;
  }

  interval_ {|aInterval, recreate=true|
    interval = aInterval;
    if(recreate) {
      this.recreateHandles;
    };
    ^this;
  }

  color_ {|aColor|
    color = aColor;
    view.refresh;
    ^this;
  }

  enabledColor_ {|aColor|
    enabledColor = aColor;
    view.refresh;
    ^this;
  }

  alpha_ {|aAlpha|
    alpha = aAlpha;
    view.refresh;
    ^this;
  }

  enabled_ {|aEnabled|
    enabled = aEnabled;

    handles.do {|handle| handle.enabled = enabled; };

    if(enabled) {
      lineWidth = 4;
    } {
      lineWidth = 1.5;
    }
    ^this;
  }

  visible_ {|aVisible|
    visible = aVisible;
    view.visible_(visible);
    ^this;
  }

  state {
    ^IdentityDictionary[
      'interval' -> interval,
      'data' -> data,
      'color' -> color
    ]
  }

  state_ {|state|
    this.data_(state['data'], false)
        .interval_(state['interval'], false)
        .color_(state['color'], false);

    this.recreateHandles;
    ^this;
  }

  env {|segTime=0.05, curves=\lin|
    ^Env(envLevels, this.envTimes, curves);
  }

  updateAction_ {|aUpdateAction|
    updateAction = aUpdateAction;
    this.setHandleActions;
    ^this;
  }
}
