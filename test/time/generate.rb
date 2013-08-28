gem "rmagick"
require "RMagick"

WIDTH = 8096
HEIGHT = 8096

def random_color()
  chars = "0123456789abcdef"
  result = "#"
  3.times { result << chars[rand(16)] }
  return result
end


image = Magick::Image.new(WIDTH, HEIGHT) do
  self.background_color = "#ffffff"
end

image.alpha = Magick::ActivateAlphaChannel
image.matte = true

draw = Magick::Draw.new
draw.stroke_width(1)

WIDTH.times do |x|
  draw.stroke(random_color)
  draw.line(x, 0, x, HEIGHT - 1)
  puts "vertical line at #{x}"
end

draw.draw(image)
image.write("vertical.bmp")

draw = Magick::Draw.new
draw.stroke_width(1)

HEIGHT.times do |y|
  draw.stroke(random_color)
  draw.line(0, y, WIDTH - 1, y)
  puts "horizontal line at #{y}"
end

draw.draw(image)
image.write("horizontal.bmp")

puts "horizontal written"

draw = Magick::Draw.new
draw.stroke_width(1)

puts "random"

WIDTH.times do |x|
  puts "random point in line #{x}"

  HEIGHT.times do |y|
    draw.fill(random_color)
    draw.point(x, y)
  end
end

draw.draw(image)
image.write("random.bmp")
