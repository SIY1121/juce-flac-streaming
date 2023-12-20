import { style } from '@vanilla-extract/css';

export const bar = style({
    position: "absolute",
    background: "yellow",
    inset: 0,
    transition: "transform 0.1s linear"
})

export const barContainer = style({
    position: 'relative',
    height: "100%",
    flex: 1,
    overflow: "hidden",
    background: "gray"
})

export const container = style({
    display: "flex",
    gap: 16,
    width: 200,
    height: 400
})