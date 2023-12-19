import { bar, barContainer, container } from "./volume.css"

const Bar: React.FC<{percent: number}> = ({percent}) => {
    return (
        <div className={barContainer}>
            <div className={bar} style={{transform: `translateY(${(1 - percent) * 100}%)`}} ></div>
        </div>
    )
}

const min = -30
const max = 0

export const Volume: React.FC<{volumes: number[]}> = ({volumes}) => {
    return <div className={container}>
        {
            volumes.map(v => {
                if(v < min) v = min
                if(v > max) v = max
                return (v - min) / (max - min)
            }).map((v, i) => <Bar key={i} percent={v}/>)
        }
    </div>
}