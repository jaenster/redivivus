return {
  server = function(game)
    game.on("tick", function(e)
      if e.tick >= 2 then
        while true do end
      end
    end)
  end,
}
