#!/usr/bin/env runhaskell

import Text.Pandoc.JSON

main :: IO ()
main = toJSONFilter suppressAudioTable

suppressAudioTable :: Block -> Block
suppressAudioTable (Div attr@(id, _, _) _) | id == "audio_table" = Null
suppressAudioTable x = x
