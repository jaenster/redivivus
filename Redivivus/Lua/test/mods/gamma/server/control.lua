local ok, err = pcall(require, "__alpha__/shared/helper")
log("gamma requires alpha without the dependency:", ok, err)
script.on_event("on_tick", function(e)
  gamma_ticks = (gamma_ticks or 0) + 1
end)
