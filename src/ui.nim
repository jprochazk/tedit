
type UIState* = object
    show_demo*: bool
    somefloat*: float32
    counter*: int32

proc newUIState*(): UIState =
    return UIState(
        show_demo: false,
        somefloat: 0.0f,
        counter: 0,
    )
