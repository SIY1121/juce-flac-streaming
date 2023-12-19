import { useCallback, useEffect, useState } from "react";
import { Player } from "./player";

export function usePlayer() {
    const [status, setStatus] = useState<"loading" | "ready" | "playing">("loading")

    const [peakVolumes, setPeakVolumes] = useState<number[]>([])
    const [player, setPlayer] = useState<Player>()

    useEffect(() => {
        const player = new Player((player) => {
            player.initDecoder()
            setStatus("ready")
        })
        player.onInfo = (pv) => {
            setPeakVolumes(pv)
        }
        setPlayer(player)
        return () => {
            player.stop()
        }
    }, [])

    const start = useCallback(() => {
        player?.start()
        player?.unlockAudio()
        setStatus("playing")
    }, [player, setStatus])

    return { start, peakVolumes, status }
}