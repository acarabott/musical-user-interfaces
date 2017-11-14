MUIHandle {
  classvar <>offColor;
  classvar <>overColor;
  classvar <>downColor;

  var <>view;
  var <>extent;
  var <>parent;
  var <center;
  var <>enabled;
  var <>grabPoint;
  var <>action;
  var <>color;

  *initClass {
    offColor =  Color(0.17, 0.61, 0.83, 0.8);
    downColor = Color(0.57, 1.0, 1.0, 0.8);
    overColor = Color(0.37, 0.81, 1.0, 0.8);
  }

  *new {|parent, center, enabled=true|
    ^super.new.init(parent, center, enabled);
  }

  init {|aParent, aCenter, aEnabled|
    enabled = aEnabled;
    extent = 20@20;
    parent = aParent;
    center = aCenter;
    this.createView;
  }

  createView {
    color = offColor;
    view = UserView(parent, Rect().extent_(extent).center_(center))
      .mouseEnterAction_({|view|
        color = overColor;
        view.refresh
      })
      .mouseLeaveAction_({|view|
        color = offColor;
        view.refresh;
      })
      .mouseDownAction_({|view, x, y, modifiers|
        grabPoint = x@y;
        color = downColor;
        view.refresh;
      })
      .mouseMoveAction_({|view, x, y, modifiers|
        var newCenter, normPoint;
        newCenter = view.bounds.center + ((x@y) - grabPoint);
        // TODO temp, only allowing y movement for now...
        // newCenter.x = newCenter.x.clip(0, view.parent.bounds.width);
        newCenter.x = center.x;
        newCenter.y = newCenter.y.clip(0, view.parent.bounds.height);

        this.center = newCenter;

        normPoint = newCenter / parent.bounds.extent;
        normPoint.y = 1 - normPoint.y;
        action.(normPoint);
      })
      .drawFunc_({|view|
        if(enabled) {
          var size = view.bounds.size;
          var center = (size.width * 0.5)@(size.height * 0.5);
          var width = 4;
          var radius = (min(size.width, size.height) * 0.5) - (width / 2.0);
          Pen.color = color;
          Pen.width = width;
          Pen.addArc(center, radius, pi, 2pi);
          Pen.perform(if (color == offColor) { \stroke } { \fill });
        };
      });
  }

  center_ {|aCenter|
    center = aCenter;
    view.bounds = view.bounds.center_(center);
    ^this;
  }

  remove {
    view.remove;
  }
}
