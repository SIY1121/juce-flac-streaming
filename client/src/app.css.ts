import { style } from '@vanilla-extract/css';

export const container = style({
    width: "100vw",
    height: "100vh",
    background: "linear-gradient(180deg, #262733 0%, #000 100%)",
    color: "white",
    display: "flex",
    flexDirection: "column",
    justifyContent: "space-evenly",
    alignItems: "center"
});

export const title = style({
    fontWeight: "bold",
    fontSize: "1.5em",
    textAlign: "center"
})

export const playButton = style({
    ":disabled": {
        opacity: 0.5
    },
    ":focus": {
        background: "gray"
    },
    padding: "1em 2em",
    border: "1px white solid",
    borderRadius: "32px"
})